#!/usr/bin/env -S uv run
# /// script
# dependencies = ["pyyaml", "loguru", "rich", "pathos", "pydantic", "fire"]
# ///

import configparser
import json
import multiprocessing
import subprocess
from collections.abc import Sequence
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Any

import fire
import yaml
from loguru import logger
from pathos.pools import ProcessPool
from plugins_utils import FilterInfo, PropertyDict, UIInfo
from pydantic import BaseModel
from rich.console import Console
from rich.progress import Progress

console = Console()

# Configure logger
logger.add("plugins_iflmake.log", rotation="1 MB")

# Constants
IBP_PATH = Path.cwd() / "build" / "build" / "ibp"

# Type aliases for better readability
PropertyDict = dict[str, "PropertyInfo"]
UIInfo = dict[str, dict[str, float]]


@dataclass
class ProcessingTask:
    """Represents a single image processing task.

    Attributes:
        input_path: Path to the input image file
        output_path: Path where the processed image should be saved
        ifl_path: Path to the .ifl filter configuration file
        plugin_name: Short identifier of the plugin
    """

    input_path: Path
    output_path: Path
    ifl_path: Path
    plugin_name: str


class PropertyType(str, Enum):
    """Supported property types in the filter plugins."""

    BOOL = "bool"
    INT = "int"
    DOUBLE = "double"
    COLOR = "color"
    ENUM = "enum"
    STRING = "string"


class EnumEncoder(json.JSONEncoder):
    """JSON encoder that handles Enum values."""

    def default(self, o: Any) -> Any:
        if isinstance(o, Enum):
            return o.value
        return super().default(o)


class PropertyInfo(BaseModel):
    """Information about a filter property."""

    name: str
    type: PropertyType
    default_value: int | float | str
    interesting_value: int | float | str
    description: str = ""
    comment: str = ""  # Added field for property-specific comments
    min_value: int | float | None = None
    max_value: int | float | None = None
    enum_values: list[str] | None = None


class ConfigError(Exception):
    """Raised when there's an error in the configuration."""


class PathosPool:
    """Context manager for Pathos parallel processing pools."""

    def __init__(self, nodes: int | None = None):
        self.nodes = nodes if nodes is not None else multiprocessing.cpu_count()
        self.pool: ProcessPool | None = None

    def __enter__(self) -> ProcessPool:
        self.pool = ProcessPool(nodes=self.nodes)
        return self.pool

    def __exit__(self, exc_type: Any, exc_value: Any, traceback: Any) -> bool:
        if self.pool:
            self.pool.close()
            self.pool.join()
            self.pool.clear()
        return False


def create_config_parser() -> configparser.ConfigParser:
    """Create and configure a ConfigParser instance.

    Returns:
        ConfigParser instance with case-sensitive options
    """
    config = configparser.ConfigParser()
    config.optionxform = lambda x: x  # type: ignore
    return config


def validate_yaml_data(data: dict) -> None:
    """Validate the YAML data structure.

    Args:
        data: Dictionary containing the YAML data

    Raises:
        ConfigError: If required fields are missing or invalid
    """
    try:
        FilterInfo(**data)
    except Exception as e:
        raise ConfigError(f"Invalid filter configuration: {e}")


def load_yaml_config(yaml_path: Path) -> FilterInfo:
    """Load and parse filter configuration from YAML file.

    Args:
        yaml_path: Path to the YAML configuration file

    Returns:
        FilterInfo containing the parsed configuration

    Raises:
        ConfigError: If the YAML file is invalid or missing required fields
        OSError: If there are file access issues
    """
    try:
        with yaml_path.open("r") as f:
            data = yaml.safe_load(f)
    except yaml.YAMLError as e:
        raise ConfigError(f"Invalid YAML format in {yaml_path}: {e}")
    except OSError as e:
        raise ConfigError(f"Failed to read {yaml_path}: {e}")

    try:
        return FilterInfo(**data)
    except Exception as e:
        raise ConfigError(f"Invalid filter configuration in {yaml_path}: {e}")


def create_ifl_file(
    filter_config: FilterInfo, output_dir: Path, write_file: bool = True
) -> str | None:
    """Create .ifl file for the filter and return its contents as string.

    Args:
        filter_config: FilterInfo containing the filter configuration
        output_dir: Directory where the .ifl file should be created
        write_file: Whether to write the IFL file to disk (True) or just return contents (False)

    Returns:
        String containing the .ifl file contents, or None if creation fails

    Raises:
        OSError: If there are file access issues when writing file
    """
    config = create_config_parser()

    # Set up filter section
    config.add_section("imageFilter1")
    config.set("imageFilter1", "id", filter_config.id)
    config.set("imageFilter1", "bypass", "false")

    # Add properties using example values if present, otherwise interesting_value
    for prop_name, prop_data in filter_config.properties.items():
        value = (
            str(filter_config.example[prop_name])
            if filter_config.example and prop_name in filter_config.example
            else str(prop_data.interesting_value)
        )
        config.set("imageFilter1", prop_name, value)

    # Add info section
    config.add_section("info")
    config.set("info", "description", filter_config.description)
    config.set("info", "fileType", "ibp.imagefilterlist")
    config.set("info", "nFilters", "1")
    config.set("info", "name", filter_config.name)

    # Get contents as string
    from io import StringIO

    output = StringIO()
    config.write(output, space_around_delimiters=False)
    contents = output.getvalue()
    output.close()

    if write_file:
        # Write to file
        ifl_path = output_dir / f"{filter_config.id.split('.')[-1]}.ifl"
        try:
            with ifl_path.open("w") as f:
                f.write(contents)
        except OSError as e:
            logger.error(f"Failed to write .ifl file {ifl_path}: {e}")
            return None

    return contents


def get_image_files(input_dir: Path) -> list[Path]:
    """Get list of supported image files from directory.

    Args:
        input_dir: Directory containing input images

    Returns:
        List of Path objects for supported image files
    """
    image_patterns = ["*.jpg", "*.jpeg", "*.png"]
    image_files: list[Path] = []

    for pattern in image_patterns:
        image_files.extend(input_dir.glob(pattern.lower()))
        image_files.extend(input_dir.glob(pattern.upper()))

    return sorted(image_files)


def process_single_image(task: ProcessingTask, force: bool = False) -> tuple[bool, str]:
    """Process a single image with the given IFL file.

    Args:
        task: Processing task containing input/output paths
        force: Whether to overwrite existing output files
    """
    try:
        # Skip if output exists and force is False
        if not force and task.output_path.exists():
            logger.info(f"Skipping existing file: {task.output_path.name}")
            return True, task.output_path.name

        logger.info(
            f"APPLYING '{task.plugin_name}': {task.input_path.name} -> {task.output_path.name} using "
        )

        subprocess.run(
            [
                str(IBP_PATH),
                "-i",
                str(task.input_path),
                "-o",
                str(task.output_path),
                "-l",
                str(task.ifl_path),
            ],
            check=True,
            capture_output=True,
            text=True,
        )
        return True, task.output_path.name
    except subprocess.CalledProcessError as e:
        logger.error(f"Process error: {e.stderr}")
        return False, task.output_path.name
    except Exception as e:
        logger.error(f"Unexpected error processing {task.output_path.name}: {e}")
        return False, task.output_path.name


def process_tasks(
    tasks: Sequence[ProcessingTask],
    progress: Progress,
    parallel: bool = False,
    force: bool = False,
) -> list[bool]:
    """Process all tasks either sequentially or in parallel."""
    task_progress = progress.add_task("[cyan]Processing images...", total=len(tasks))
    results = []

    if parallel:
        with PathosPool() as pool:
            for success, output_file in pool.imap(
                lambda t: process_single_image(t, force), tasks
            ):
                results.append(success)
                logger.info(f"Processed: {output_file}")
                progress.update(task_progress, advance=1)
    else:
        for task in tasks:
            success, output_file = process_single_image(task, force)
            results.append(success)
            logger.info(f"Processed: {output_file}")
            progress.update(task_progress, advance=1)

    return results


def create_markdown(
    filter_config: FilterInfo,
    ifl_contents: str,
    input_images: list[Path],
    output_dir: Path,
) -> None:
    """Create markdown documentation file for the filter.

    Args:
        filter_config: FilterInfo containing the filter configuration
        ifl_contents: String containing the .ifl file contents
        input_images: List of input image paths
        output_dir: Directory where the markdown file should be created

    Raises:
        OSError: If there are file access issues
    """
    shortid = filter_config.id.split(".")[-1]
    image_table_rows = []
    for img_path in input_images:
        input_basename = img_path.stem
        output_basename = f"{input_basename}_{shortid}"
        output_ext = img_path.suffix.lower()  # Normalize extension case

        row = (
            f"| ![{input_basename}](../assets/img_in/{img_path.name}) "
            f"| ![{output_basename}](../assets/img_out/{output_basename}{output_ext}) |"
        )
        image_table_rows.append(row)

    markdown_content = f"""# {shortid}

{filter_config.description}

| Input | Output |
|--------|--------|
{chr(10).join(image_table_rows)}

### Configuration

```ini
{ifl_contents}
```
"""

    md_path = output_dir / f"{shortid}.md"
    try:
        md_path.write_text(markdown_content)
    except OSError as e:
        logger.error(f"Failed to write markdown file {md_path}: {e}")
        raise


def generate_documentation(
    yaml_files: list[Path],
    input_images: list[Path],
    plugins_output_dir: Path,
    list_force: bool = True,
) -> None:
    """
    Process each filter.yaml file to create the corresponding .ifl and markdown
    documentation.

    Args:
        yaml_files: List of YAML configuration files to process
        input_images: List of input image paths
        plugins_output_dir: Directory where to output plugin documentation
        list_force: Whether to write IFL files to disk (True) or just generate contents (False)
    """
    for yaml_path in yaml_files:
        try:
            logger.info(f"Processing documentation for {yaml_path}")
            filter_config = load_yaml_config(yaml_path)
            ifl_contents = create_ifl_file(
                filter_config, plugins_output_dir, write_file=list_force
            )
            if ifl_contents is None:
                continue
            create_markdown(
                filter_config, ifl_contents, input_images, plugins_output_dir
            )
            logger.info(f"Created documentation for: {filter_config.id.split('.')[-1]}")
        except ConfigError as e:
            logger.error(f"Configuration error in {yaml_path}: {e}")
        except Exception as e:
            logger.error(
                f"Unexpected error in documentation generation for {yaml_path}: {e}"
            )


def run_image_processing(
    yaml_files: list[Path],
    input_images: list[Path],
    docs_dir: Path,
    output_img_dir: Path,
    parallel: bool,
    force: bool,
) -> None:
    """
    Create processing tasks for images and process them either sequentially
    or in parallel.
    """
    tasks = create_processing_tasks(yaml_files, input_images, output_img_dir, docs_dir)
    if not tasks:
        logger.error("No processing tasks created")
        return

    logger.info(f"Starting image processing for {len(tasks)} tasks")
    with Progress() as progress:
        results = process_tasks(tasks, progress, parallel, force)

    successful = sum(results)
    failed = len(results) - successful
    console.print(f"\nProcessing complete: {successful} successful, {failed} failed")


def create_processing_tasks(
    yaml_files: Sequence[Path],
    input_images: Sequence[Path],
    output_dir: Path,
    docs_dir: Path,
) -> list[ProcessingTask]:
    """Create processing tasks from input files and YAML files."""
    tasks = []
    for yaml_path in yaml_files:
        try:
            filter_config = load_yaml_config(yaml_path)
            shortid = filter_config.id.split(".")[-1]
            ifl_path = docs_dir / "plugins" / f"{shortid}.ifl"

            for input_image in input_images:
                output_path = (
                    output_dir
                    / f"{input_image.stem}_{shortid}{input_image.suffix.lower()}"
                )
                tasks.append(
                    ProcessingTask(
                        input_path=input_image,
                        output_path=output_path,
                        ifl_path=ifl_path,
                        plugin_name=shortid,
                    )
                )
        except Exception as e:
            logger.error(f"Error creating task for {yaml_path}: {e}")
            continue

    return tasks


def main(
    parallel: bool = False,
    force: bool = False,
    images: bool = False,
    markdown: bool = False,
    list_force: bool = False,
) -> None:
    """Process filter plugins and generate documentation.

    Args:
        parallel: Whether to process images in parallel
        force: Whether to overwrite existing output files
        images: Whether to generate output images
        markdown: Whether to generate markdown documentation
        list_force: Whether to overwrite IFL files in docs/plugins (if False, IFL files are read but not overwritten)
    """
    # Setup paths
    base_dir = Path.cwd()
    plugins_dir = base_dir / "src" / "plugins"
    docs_dir = base_dir / "docs"
    plugins_output_dir = docs_dir / "plugins"
    input_img_dir = docs_dir / "assets" / "img_in"
    output_img_dir = docs_dir / "assets" / "img_out"

    logger.info("Starting plugin documentation generation")

    # Ensure necessary directories exist
    try:
        if markdown:
            plugins_output_dir.mkdir(parents=True, exist_ok=True)
        if images:
            output_img_dir.mkdir(parents=True, exist_ok=True)
    except OSError as e:
        logger.error(f"Failed to create output directories: {e}")
        return

    # Get list of input images
    input_images = get_image_files(input_img_dir)
    if not input_images:
        logger.warning(f"No supported image files found in {input_img_dir}")
        return
    logger.info(f"Found {len(input_images)} input images")

    # Discover filter configuration YAML files
    yaml_files = list(plugins_dir.glob("**/filter.yaml"))
    if not yaml_files:
        logger.warning(f"No filter.yaml files found in {plugins_dir}")
        return
    logger.info(f"Found {len(yaml_files)} filter configurations")

    # Documentation generation pass
    if markdown:
        generate_documentation(
            yaml_files, input_images, plugins_output_dir, list_force=list_force
        )

    # Image processing pass
    if images:
        run_image_processing(
            yaml_files, input_images, docs_dir, output_img_dir, parallel, force
        )

    logger.info("Plugin documentation generation completed")


if __name__ == "__main__":
    fire.Fire(main)

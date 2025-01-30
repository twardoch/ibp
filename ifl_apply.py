#!/usr/bin/env python3

import configparser
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Literal, Optional, Sequence, Tuple, TypeVar

import fire
from loguru import logger
from pathos.helpers import mp
from pathos.pools import ProcessPool
from rich.console import Console
from rich.progress import Progress

T = TypeVar("T")
U = TypeVar("U")

console = Console()


@dataclass
class ProcessingTask:
    """Represents a single image processing task."""

    input_path: Path
    output_path: Path
    ifl_path: Path
    plugin_name: str


class PathosPool:
    """Context manager for Pathos parallel processing pools."""

    def __init__(self, nodes: Optional[int] = None):
        self.nodes = nodes if nodes is not None else mp.cpu_count()
        self.pool: Optional[ProcessPool] = None

    def __enter__(self) -> ProcessPool:
        self.pool = ProcessPool(nodes=self.nodes)
        return self.pool

    def __exit__(self, exc_type: Any, exc_value: Any, traceback: Any) -> Literal[False]:
        if self.pool:
            self.pool.close()
            self.pool.join()
            self.pool.clear()
        return False


def setup_logging(log_dir: Path) -> None:
    """Configure logging with file and console outputs."""
    logger.remove()  # Remove default handler
    log_file = log_dir / "ifl_apply.log"

    logger.add(
        str(log_file),
        rotation="10 MB",
        retention="1 week",
        level="INFO",
        format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}",
    )
    logger.add(
        lambda msg: console.print(msg, highlight=False),
        level="INFO",
        format="{message}",
    )


def find_files(base_path: Path, pattern: str) -> List[Path]:
    """Find files matching pattern in base_path."""
    if not base_path.exists():
        logger.error(f"Directory not found: {base_path}")
        return []

    return list(base_path.glob(pattern))


def get_plugin_name(ifl_path: Path) -> str:
    """Extract plugin name from imagefilter directory name."""
    parent_dir = ifl_path.parent.name
    if parent_dir.startswith("imagefilter_"):
        return parent_dir.removeprefix("imagefilter_")
    return ifl_path.stem


def create_processing_tasks(
    example_files: Sequence[Path], input_images: Sequence[Path], output_dir: Path
) -> List[ProcessingTask]:
    """Create processing tasks from input files and IFL files."""
    tasks = []
    for ifl_file in example_files:
        plugin_name = get_plugin_name(ifl_file)
        for input_image in input_images:
            output_path = output_dir / f"{input_image.stem}_{plugin_name}.jpg"
            tasks.append(
                ProcessingTask(
                    input_path=input_image,
                    output_path=output_path,
                    ifl_path=ifl_file,
                    plugin_name=plugin_name,
                )
            )
    return tasks


def process_single_image(task: ProcessingTask) -> Tuple[bool, str]:
    """Process a single image with the given IFL file."""
    try:
        ibp_path = Path(__file__).parent / "build/ibp"
        result = subprocess.run(
            [
                str(ibp_path),
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
    verbose: bool = False,
    fast: bool = False,
) -> List[bool]:
    """Process all tasks either sequentially or in parallel."""
    task_progress = progress.add_task("[cyan]Processing...", total=len(tasks))
    results = []

    if fast:
        with PathosPool() as pool:
            for success, output_file in pool.imap(process_single_image, tasks):
                results.append(success)
                if verbose and success:
                    console.print(f"Successfully processed {output_file}")
                progress.update(task_progress, advance=1)
    else:
        for task in tasks:
            success, output_file = process_single_image(task)
            results.append(success)
            if verbose and success:
                console.print(f"Successfully processed {output_file}")
            progress.update(task_progress, advance=1)

    return results


def print_summary(results: Sequence[bool], total: int) -> None:
    """Print processing results summary."""
    processed = sum(results)
    failed = total - processed

    console.print("\n[bold blue]Processing Summary:[/]")
    console.print(f"  • Successfully processed: {processed}/{total}")
    if failed:
        console.print(f"  • [red]Failed: {failed}[/]")


def read_ifl_file(ifl_path: Path) -> Dict[str, Dict[str, str]]:
    """Read and parse an IFL file."""
    config = configparser.ConfigParser()
    config.read(str(ifl_path))
    return {section: dict(config[section]) for section in config.sections()}


def create_markdown_table(
    input_images: Sequence[Path], tasks: Sequence[ProcessingTask]
) -> str:
    """Create a markdown table showing input and output images for a filter."""
    table = "| Input | Output |\n|--------|--------|\n"

    # Group tasks by plugin name
    plugin_tasks: Dict[str, List[ProcessingTask]] = {}
    for task in tasks:
        if task.plugin_name not in plugin_tasks:
            plugin_tasks[task.plugin_name] = []
        plugin_tasks[task.plugin_name].append(task)

    # Create table rows
    for input_image in input_images:
        for plugin_name, tasks in plugin_tasks.items():
            task = next((t for t in tasks if t.input_path == input_image), None)
            if task:
                rel_input = Path("img_in") / input_image.name
                rel_output = Path("img_out") / task.output_path.name
                table += f"| ![{input_image.stem}]({rel_input}) | ![{task.output_path.stem}]({rel_output}) |\n"

    return table


def create_index_md(
    tasks: Sequence[ProcessingTask], input_images: Sequence[Path], doc_dir: Path
) -> None:
    """Create index.md file with filter documentation."""
    index_content = "# Image Batch Processor Filters\n\n"

    # Group tasks by plugin
    plugin_tasks: Dict[str, List[ProcessingTask]] = {}
    for task in tasks:
        if task.plugin_name not in plugin_tasks:
            plugin_tasks[task.plugin_name] = []
        plugin_tasks[task.plugin_name].append(task)

    # Create sections for each filter
    for plugin_name, tasks in plugin_tasks.items():
        if not tasks:
            continue

        # Read filter info from IFL file
        ifl_data = read_ifl_file(tasks[0].ifl_path)
        filter_name = ifl_data.get("info", {}).get("name", plugin_name.title())
        filter_desc = ifl_data.get("info", {}).get(
            "description", f"Filter for {plugin_name}"
        )

        # Add filter section
        index_content += f"\n## {filter_name}\n\n"
        index_content += f"{filter_desc}\n\n"

        # Add image comparison table
        index_content += create_markdown_table(input_images, tasks)

        # Add IFL configuration
        index_content += "\n### Configuration\n\n"
        index_content += "```ini\n"
        with tasks[0].ifl_path.open() as f:
            index_content += f.read()
        index_content += "```\n"

    # Write the index.md file
    index_path = doc_dir / "index.md"
    index_path.write_text(index_content)
    console.print(f"\n[green]Created documentation:[/] {index_path}")


def process_images(verbose: bool = False, fast: bool = False) -> None:
    """Process all combinations of input images and IFL files."""
    script_dir = Path(__file__).parent.resolve()
    setup_logging(script_dir)

    # Find input files
    example_files = find_files(script_dir / "src/plugins", "imagefilter_*/example.ifl")
    if not example_files:
        logger.error("No example.ifl files found")
        return

    input_images = find_files(script_dir / "doc/img_in", "*.jpg")
    if not input_images:
        logger.error("No input images found")
        return

    # Setup output directory
    doc_dir = script_dir / "doc"
    output_dir = doc_dir / "img_out"
    output_dir.mkdir(exist_ok=True)

    # Create and process tasks
    tasks = create_processing_tasks(example_files, input_images, output_dir)
    console.print("\n[bold green]Starting image processing...[/]")

    with Progress() as progress:
        results = process_tasks(tasks, progress, verbose, fast)
        print_summary(results, len(tasks))

    # Create documentation
    create_index_md(tasks, input_images, doc_dir)


def main(verbose: bool = False, fast: bool = False) -> None:
    """Process images with IFL filters."""
    try:
        process_images(verbose=verbose, fast=fast)
    except Exception as e:
        logger.exception(f"Fatal error: {e}")
        raise


if __name__ == "__main__":
    fire.Fire(main)

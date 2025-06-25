#!/usr/bin/env -S uv run
# /// script
# dependencies = ["fire", "loguru", "tqdm"]
# ///

"""
Plugin tuner tool for IBP (Image Batch Processor).
Helps tune filter parameters by running IBP and updating IFL files with user-adjusted settings.
"""

import configparser
import datetime
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path

import fire
from loguru import logger
from tqdm import tqdm

# Configure logging
logger.remove()  # Remove default handler
logger.add(sys.stderr, format="{time} | {level} | {message}", level="INFO")
logger.add("plugins_tuner.log", rotation="1 MB", level="INFO")


@dataclass
class AppPaths:
    """Container for application-related paths."""

    ibp_executable: Path
    settings_file: Path
    default_image: Path
    plugins_dir: Path

    @classmethod
    def from_cwd(cls) -> "AppPaths":
        """Create AppPaths instance from current working directory."""
        cwd = Path.cwd()
        return cls(
            ibp_executable=cwd / "build" / "build" / "ibp",
            settings_file=cwd
            / "build"
            / "build"
            / "settings"
            / "imagebatchprocessor.ifl",
            default_image=cwd / "docs" / "assets" / "img_in" / "female.jpg",
            plugins_dir=cwd / "docs" / "plugins",
        )


class IFLConfig:
    """Handler for IFL configuration files."""

    def __init__(self, config: configparser.ConfigParser):
        self.config = config

    @classmethod
    def create_empty(cls) -> "IFLConfig":
        """Create a new IFLConfig with empty ConfigParser."""
        config = configparser.ConfigParser()
        config.optionxform = lambda x: x  # type: ignore
        return cls(config)

    @classmethod
    def from_file(cls, path: Path) -> "IFLConfig":
        """Read and parse an IFL file.

        Args:
            path: Path to the IFL file

        Raises:
            FileNotFoundError: If file doesn't exist
            configparser.Error: If parsing fails
        """
        config = cls.create_empty()
        try:
            with path.open("r", encoding="utf-8") as f:
                config.config.read_file(f)
            return config
        except Exception as e:
            logger.error(f"Failed to read IFL file {path}: {e}")
            raise

    def write_to_file(self, path: Path) -> None:
        """Write configuration to an IFL file.

        Args:
            path: Path where to write the file

        Raises:
            OSError: If writing fails
        """
        try:
            with path.open("w", encoding="utf-8") as f:
                self.config.write(f, space_around_delimiters=False)
        except Exception as e:
            logger.error(f"Failed to write IFL file {path}: {e}")
            raise

    def update_from(self, other: "IFLConfig") -> None:
        """Update configuration from another IFLConfig.

        Args:
            other: Source IFLConfig to copy settings from
        """
        for section in other.config.sections():
            if not section.startswith("imageFilter"):
                continue
            if section not in self.config.sections():
                continue
            for key, value in other.config[section].items():
                if key != "id":  # Preserve original filter ID
                    self.config[section][key] = value


class IBPRunner:
    """Handles IBP application execution and settings management."""

    def __init__(self, paths: AppPaths):
        self.paths = paths

    def run_with_config(self, input_image: Path, ifl_path: Path) -> None:
        """Run IBP with specified input image and IFL file.

        Args:
            input_image: Path to input image
            ifl_path: Path to IFL configuration file

        Raises:
            subprocess.CalledProcessError: If IBP execution fails
            FileNotFoundError: If required files don't exist
        """
        try:
            subprocess.run(
                [
                    str(self.paths.ibp_executable),
                    "-i",
                    str(input_image),
                    "-l",
                    str(ifl_path),
                ],
                check=True,
            )
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to run IBP: {e}")
            raise
        except FileNotFoundError as e:
            logger.error(f"IBP executable or input files not found: {e}")
            raise

    def update_settings(self, original_ifl: Path) -> None:
        """Update original IFL file with latest settings.

        Args:
            original_ifl: Path to original IFL file

        Raises:
            FileNotFoundError: If settings file missing
            configparser.Error: If parsing fails
        """
        try:
            original_config = IFLConfig.from_file(original_ifl)
            settings_config = IFLConfig.from_file(self.paths.settings_file)

            original_config.update_from(settings_config)
            original_config.write_to_file(original_ifl)

            logger.info(f"Updated {original_ifl} with new settings")
        except Exception as e:
            logger.error(f"Failed to update IFL file with settings: {e}")
            raise


def find_ifl_files(plugins_dir: Path) -> list[Path]:
    """Find all IFL files in the plugins directory.

    Args:
        plugins_dir: Directory to search for IFL files

    Returns:
        List of paths to IFL files
    """
    return sorted(plugins_dir.glob("*.ifl"))


def is_file_older_than_hours(file_path: Path, hours: int = 3) -> bool:
    """Check if a file was last modified more than specified hours ago.

    Args:
        file_path: Path to the file to check
        hours: Number of hours to compare against

    Returns:
        True if file is older than specified hours, False otherwise
    """
    if not file_path.exists():
        return True

    mtime = datetime.datetime.fromtimestamp(file_path.stat().st_mtime)
    age = datetime.datetime.now() - mtime
    return age > datetime.timedelta(hours=hours)


def process_single_file(
    runner: IBPRunner, ifl_path: Path, input_image: Path, skip_recent: bool = False
) -> None:
    """Process a single IFL file.

    Args:
        runner: IBPRunner instance
        ifl_path: Path to IFL file
        input_image: Path to input image
        skip_recent: If True, skip files modified in the last 3 hours

    Raises:
        Exception: If processing fails
    """
    if skip_recent and not is_file_older_than_hours(ifl_path):
        logger.info(f"Skipping {ifl_path.name} - modified within last 3 hours")
        return

    logger.info(f"Processing {ifl_path.name}")

    runner.run_with_config(input_image, ifl_path)
    time.sleep(1)  # Ensure settings file is written

    if runner.paths.settings_file.exists():
        runner.update_settings(ifl_path)
    else:
        logger.warning("Settings file not found after IBP execution")


def process_files_with_progress(
    runner: IBPRunner,
    ifl_files: list[Path],
    input_image: Path,
    skip_recent: bool = False,
) -> None:
    """Process IFL files with progress tracking.

    Args:
        runner: IBPRunner instance
        ifl_files: List of IFL files to process
        input_image: Path to input image
        skip_recent: If True, skip files modified in the last 3 hours
    """
    for ifl_file in tqdm(ifl_files, desc="Processing IFL files", unit="file"):
        try:
            process_single_file(runner, ifl_file, input_image, skip_recent=skip_recent)
            tqdm.write(f"✓ Processed {ifl_file.name}")
        except Exception as e:
            logger.error(f"Failed to process {ifl_file.name}: {e}")
            tqdm.write(f"✗ Failed {ifl_file.name}: {str(e)}")


def validate_paths(input_image: Path, plugins_dir: Path) -> bool:
    """Validate required paths exist.

    Args:
        input_image: Path to input image
        plugins_dir: Path to plugins directory

    Returns:
        True if all paths are valid, False otherwise
    """
    if not input_image.exists():
        print(f"Error: Input image not found: {input_image}", file=sys.stderr)
        return False

    if not plugins_dir.exists():
        print(f"Error: Plugins directory not found: {plugins_dir}", file=sys.stderr)
        return False

    return True


def main(
    input_image: Path | None = None,
    plugins_dir: Path | None = None,
    old: bool = False,
) -> None:
    """Process IFL files in the plugins directory.

    Args:
        input_image: Optional path to input image (defaults to female.jpg)
        plugins_dir: Optional path to plugins directory (defaults to docs/plugins)
        old: If True, skip processing IFL files modified in the last 3 hours
    """
    paths = AppPaths.from_cwd()
    input_image = input_image or paths.default_image
    plugins_dir = plugins_dir or paths.plugins_dir

    if not validate_paths(input_image, plugins_dir):
        return

    ifl_files = find_ifl_files(plugins_dir)
    if not ifl_files:
        print(f"Warning: No IFL files found in {plugins_dir}", file=sys.stderr)
        return

    print(f"Found {len(ifl_files)} IFL files to process")
    runner = IBPRunner(paths)

    try:
        process_files_with_progress(runner, ifl_files, input_image, skip_recent=old)
        print("\nPlugin tuning completed!")
    except Exception as e:
        logger.error(f"Fatal error during processing: {e}")
        print("\nPlugin tuning failed!", file=sys.stderr)
        raise


if __name__ == "__main__":
    fire.Fire(main)

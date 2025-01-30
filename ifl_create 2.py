#!/usr/bin/env python3

import configparser
import re
from pathlib import Path
from typing import Dict

# Special parameter values for specific filters
SPECIAL_FILTER_VALUES: Dict[str, Dict[str, str]] = {
    "brightnesscontrast": {
        "alphabrightnesscontrast": "0 0",
        "bluebrightnesscontrast": "0 0",
        "greenbrightnesscontrast": "0 0",
        "redbrightnesscontrast": "0 0",
        "rgbbrightnesscontrast": "17 17",
        "usesoftmode": "false",
        "workingchannel": "rgb",
    },
    "autolevels": {
        "enhancechannelsseparately": "false",
        "adjustmidtones": "false",
        "targetcolorhighlights": "#ffffff",
        "targetcolormidtones": "#808080",
        "targetcolorshadows": "#000000",
        "clippinghighlights": "0.1",
        "clippingshadows": "0.1",
    },
    "colorbalance": {
        "preserveluminosity": "false",
        "shadowsred": "0",
        "shadowsgreen": "0",
        "shadowsblue": "0",
        "midtonesred": "0",
        "midtonesgreen": "17",
        "midtonesblue": "0",
        "highlightsred": "17",
        "highlightsgreen": "0",
        "highlightsblue": "0",
    },
}


def get_default_values(param_type: str) -> str:
    """Get sensible default values based on parameter type."""
    defaults = {
        "bool": "true",
        "int": "3",
        "double": "0.3",
        "const QColor &": "#FF0000",
        "Filter::WorkingChannel": "rgb",
        "Filter::Position": "front",
        "Filter::Direction": "horizontal",
        "Filter::Angle": "90clockwise",
        "Filter::InterpolationMode": "linear",
        "Filter::OutputMode": "normal",
        "Filter::Distribution": "uniform",
        "Filter::ColorMode": "rgb",
        "Filter::SizeMode": "pixels",
        "Filter::ResamplingMode": "bicubic",
        "Filter::MorphologyOp": "erosion",
        "Filter::KernelShape": "rectangle",
        "Filter::Reference": "topleft",
        "Filter::ResizeMode": "absolute",
        "ColorCompositionMode": "normal",
        "QPainter::CompositionMode_Source": "source-over",
    }
    return defaults.get(param_type, "0")


def convert_param_name(name: str) -> str:
    """Convert parameter name to the format expected by the application."""
    # Remove common prefixes
    name = re.sub(r"^(set|get|m)", "", name)
    # Convert camelCase to lowercase without underscores
    return re.sub(r"([a-z0-9])([A-Z])", r"\1\2", name).lower()


def extract_plugin_data(main_cpp_path: Path) -> Dict[str, str]:
    """
    Extract plugin name and description from main.cpp.

    Args:
        main_cpp_path: Path to the main.cpp file

    Returns:
        Dictionary containing name and description if found
    """
    data: Dict[str, str] = {}

    try:
        content = main_cpp_path.read_text()
        name_match = re.search(
            r'info\.insert\("name",\s*QObject::tr\("(.*?)"\)\);', content
        )
        desc_match = re.search(
            r'info\.insert\("description",\s*QObject::tr\("(.*?)"\)\);', content
        )

        if name_match:
            data["name"] = name_match.group(1)
        if desc_match:
            data["description"] = desc_match.group(1)

    except FileNotFoundError:
        print(f"Warning: main.cpp not found at {main_cpp_path}")

    return data


def parse_filter_parameters(
    filter_file: Path, filter_id: str, special_values: Dict[str, Dict[str, str]]
) -> Dict[str, str]:
    """
    Parse filter parameters from filter.cpp file.

    Args:
        filter_file: Path to the filter.cpp file
        filter_id: ID of the filter
        special_values: Dictionary of special parameter values for specific filters

    Returns:
        Dictionary of parameter names and their values
    """
    parameters: Dict[str, str] = {}

    try:
        content = filter_file.read_text()
        matches = re.finditer(r"set([A-Za-z]+)\s*\(\s*([\w:&\s]+)\s+\w+\s*\)", content)

        for match in matches:
            param_name = convert_param_name(match.group(1))
            param_type = match.group(2).strip()

            # Skip if parameter has special value
            if filter_id in special_values and param_name in special_values[filter_id]:
                continue

            param_value = get_default_values(param_type)
            parameters[param_name] = param_value

    except FileNotFoundError:
        print(f"Warning: Could not read filter file: {filter_file}")

    return parameters


def create_config_parser() -> configparser.ConfigParser:
    """Create and configure a ConfigParser instance."""
    config = configparser.ConfigParser()
    # Fix the type error by explicitly typing the lambda
    config.optionxform = lambda x: x  # type: ignore
    return config


def create_ifl_files(base_dir: Path) -> None:
    """
    Create .ifl files for each image filter plugin.

    Args:
        base_dir: Base directory of the project
    """
    plugins_dir = base_dir / "src" / "plugins"

    for plugin_dir in plugins_dir.iterdir():
        if not plugin_dir.is_dir() or not plugin_dir.name.startswith("imagefilter_"):
            continue

        filter_file = plugin_dir / "filter.cpp"
        main_cpp_path = plugin_dir / "main.cpp"

        if not filter_file.is_file() or not main_cpp_path.is_file():
            continue

        filter_id = plugin_dir.name.replace("imagefilter_", "")
        ifl_file = plugin_dir / "example.ifl"

        # Create config
        config = create_config_parser()

        # Set up filter section
        config.add_section("imageFilter1")
        config.set("imageFilter1", "id", f"ibp.imagefilter.{filter_id}")
        config.set("imageFilter1", "bypass", "false")

        # Add special values if they exist
        if filter_id in SPECIAL_FILTER_VALUES:
            for key, value in SPECIAL_FILTER_VALUES[filter_id].items():
                config.set("imageFilter1", key, value)

        # Parse and add filter parameters
        parameters = parse_filter_parameters(
            filter_file, filter_id, SPECIAL_FILTER_VALUES
        )
        for param_name, param_value in parameters.items():
            config.set("imageFilter1", param_name, param_value)

        # Add plugin info
        plugin_data = extract_plugin_data(main_cpp_path)
        config.add_section("info")
        config.set(
            "info",
            "description",
            plugin_data.get(
                "description",
                f"This is a single filter configuration for {plugin_dir.name}.",
            ),
        )
        config.set("info", "fileType", "ibp.imagefilterlist")
        config.set("info", "nFilters", "1")
        config.set("info", "name", plugin_data.get("name", plugin_dir.name))

        # Write config to file
        with ifl_file.open("w") as configfile:
            config.write(configfile, space_around_delimiters=False)

        print(f"Created: {ifl_file}")


def main() -> None:
    """Entry point for the script."""
    base_dir = Path.cwd()
    create_ifl_files(base_dir)


if __name__ == "__main__":
    main()

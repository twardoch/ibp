#!/usr/bin/env python3

import configparser
import os
import re
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Dict, List, NamedTuple, Optional, Tuple

import clang.cindex
from clang.cindex import Config, Index, TranslationUnit
from loguru import logger

# Configure libclang
if os.path.exists("/usr/local/opt/llvm/lib/libclang.dylib"):
    Config.set_library_file("/usr/local/opt/llvm/lib/libclang.dylib")
elif os.path.exists("/opt/homebrew/opt/llvm/lib/libclang.dylib"):
    Config.set_library_file("/opt/homebrew/opt/llvm/lib/libclang.dylib")


# Define cursor kind values based on libclang's internal values
class CursorKinds:
    CALL_EXPR = 103
    INTEGER_LITERAL = 106
    FLOATING_LITERAL = 107
    STRING_LITERAL = 109
    CXX_BOOL_LITERAL_EXPR = 129
    CXX_METHOD = 21
    PARM_DECL = 10
    CONSTRUCTOR = 24


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
        # Updated regex patterns to handle QObject::tr calls
        name_match = re.search(
            r'info\.insert\("name",\s*QObject::tr\("([^"]+)"\)\);', content
        )
        desc_match = re.search(
            r'info\.insert\("description",\s*QObject::tr\("([^"]+)"\)\);', content
        )

        if name_match:
            data["name"] = name_match.group(1)
        if desc_match:
            data["description"] = desc_match.group(1)

    except FileNotFoundError:
        logger.warning(f"Warning: main.cpp not found at {main_cpp_path}")

    return data


def extract_plugin_data_with_clang(main_cpp_path: Path) -> Dict[str, str]:
    """
    Extract plugin name and description from main.cpp using libclang.

    Args:
        main_cpp_path: Path to the main.cpp file

    Returns:
        Dictionary containing name and description if found
    """
    data: Dict[str, str] = {}

    try:
        index = Index.create()
        tu = index.parse(
            str(main_cpp_path), args=["-x", "c++", "-std=c++17", "-I/usr/local/include"]
        )

        def find_string_literal(node: clang.cindex.Cursor) -> Optional[str]:
            """Find the first string literal in a node's children."""
            if node.kind.value == CursorKinds.STRING_LITERAL:
                tokens = list(node.get_tokens())
                if tokens:
                    return tokens[0].spelling.strip('"')
            for child in node.get_children():
                result = find_string_literal(child)
                if result:
                    return result
            return None

        for node in tu.cursor.walk_preorder():
            if node.kind.value == CursorKinds.CALL_EXPR and node.spelling == "insert":
                args = list(node.get_arguments())
                if len(args) >= 2:
                    # Get the key from the first argument
                    key_str = find_string_literal(args[0])
                    if not key_str or key_str not in ["name", "description"]:
                        continue

                    # Get the value from the second argument
                    value_str = find_string_literal(args[1])
                    if value_str:
                        data[key_str] = value_str

        if not data:
            # If libclang failed to extract the data, try the regex approach
            return extract_plugin_data(main_cpp_path)

        return data
    except Exception as e:
        logger.warning(f"Failed to parse {main_cpp_path} with libclang: {e}")
        # Fall back to regex-based extraction
        return extract_plugin_data(main_cpp_path)


class ParameterInfo(NamedTuple):
    """Information about a parameter including its type, range, and default value."""

    name: str
    param_type: str
    default_value: Optional[str] = None
    min_value: Optional[str] = None
    max_value: Optional[str] = None
    constructor_default: Optional[str] = None
    ui_default: Optional[str] = None


def extract_parameter_info_from_ui(ui_file: Path) -> Dict[str, ParameterInfo]:
    """Extract parameter information from Qt UI file."""
    params: Dict[str, ParameterInfo] = {}

    try:
        tree = ET.parse(ui_file)
        root = tree.getroot()

        # Find all QSpinBox and QDoubleSpinBox widgets
        for widget in root.findall(".//widget"):
            widget_name = widget.get("name", "")
            if not widget_name.startswith("mSpin"):
                continue

            # Convert mSpinParamName to paramname
            param_name = convert_param_name(widget_name[5:])

            # Extract min/max/default values
            min_value = max_value = default_value = None
            for prop in widget.findall("property"):
                number_elem = prop.find("number")
                if number_elem is None or number_elem.text is None:
                    continue

                if prop.get("name") == "minimum":
                    min_value = number_elem.text
                elif prop.get("name") == "maximum":
                    max_value = number_elem.text
                elif prop.get("name") == "value":
                    default_value = number_elem.text

            params[param_name] = ParameterInfo(
                name=param_name,
                param_type="int",  # Could be improved to detect double
                ui_default=default_value,
                min_value=min_value,
                max_value=max_value,
            )
    except Exception as e:
        logger.warning(f"Failed to parse UI file {ui_file}: {e}")

    return params


def extract_constructor_defaults(filter_file: Path) -> Dict[str, str]:
    """Extract default values from the constructor."""
    defaults: Dict[str, str] = {}

    try:
        index = Index.create()
        tu = index.parse(str(filter_file), args=["-x", "c++", "-std=c++17"])

        for node in tu.cursor.walk_preorder():
            if node.kind.value == CursorKinds.CONSTRUCTOR:
                # Look for member initializer list
                for child in node.get_children():
                    if child.kind.value == CursorKinds.CXX_BOOL_LITERAL_EXPR:
                        tokens = list(child.get_tokens())
                        if tokens:
                            var_name = convert_param_name(child.spelling)
                            defaults[var_name] = tokens[0].spelling
                    elif child.kind.value in [
                        CursorKinds.INTEGER_LITERAL,
                        CursorKinds.FLOATING_LITERAL,
                    ]:
                        tokens = list(child.get_tokens())
                        if tokens:
                            var_name = convert_param_name(child.spelling)
                            defaults[var_name] = tokens[0].spelling
    except Exception as e:
        logger.warning(
            f"Failed to extract constructor defaults from {filter_file}: {e}"
        )

    return defaults


def get_noticeable_default(param: ParameterInfo) -> str:
    """Generate a noticeable default value based on parameter type and range."""
    if param.param_type == "bool":
        return "true"

    # Use constructor default if available
    if param.constructor_default is not None:
        return param.constructor_default

    # Use UI default if available
    if param.ui_default is not None:
        return param.ui_default

    # For numeric types with ranges
    if param.min_value is not None and param.max_value is not None:
        min_val = float(param.min_value)
        max_val = float(param.max_value)

        # For parameters that go negative to positive
        if min_val < 0 and max_val > 0:
            # Use 25% of the positive range
            return str(int(max_val * 0.25))

        # For parameters that are just positive
        if min_val >= 0:
            # Use 25% of the range
            return str(int((max_val - min_val) * 0.25 + min_val))

    # Fallback to type-based defaults
    return get_default_values(param.param_type)


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


def parse_filter_parameters_with_clang(
    filter_file: Path,
    filter_id: str,
    special_values: Dict[str, Dict[str, str]],
    ui_file: Optional[Path] = None,
) -> Dict[str, str]:
    """Parse filter parameters using libclang with enhanced default value handling."""
    parameters: Dict[str, str] = {}

    try:
        # Get UI parameter info if available
        ui_params = {}
        if ui_file and ui_file.exists():
            ui_params = extract_parameter_info_from_ui(ui_file)

        # Get constructor defaults
        constructor_defaults = extract_constructor_defaults(filter_file)

        # Parse the filter file
        index = Index.create()
        tu = index.parse(str(filter_file), args=["-x", "c++", "-std=c++17"])

        def extract_default_value(node: clang.cindex.Cursor) -> Optional[str]:
            """Extract default value from a parameter declaration."""
            for child in node.get_children():
                if child.kind.value in [
                    CursorKinds.INTEGER_LITERAL,
                    CursorKinds.FLOATING_LITERAL,
                    CursorKinds.STRING_LITERAL,
                    CursorKinds.CXX_BOOL_LITERAL_EXPR,
                ]:
                    tokens = list(child.get_tokens())
                    if tokens:
                        return tokens[0].spelling
            return None

        for node in tu.cursor.walk_preorder():
            if node.kind.value == CursorKinds.CXX_METHOD and node.spelling.startswith(
                "set"
            ):
                param_name = convert_param_name(node.spelling)

                # Skip if parameter has special value
                if (
                    filter_id in special_values
                    and param_name in special_values[filter_id]
                ):
                    continue

                # Get parameter type and default value
                for param in node.get_arguments():
                    if param.kind.value == CursorKinds.PARM_DECL:
                        param_type = param.type.spelling

                        # Get UI parameter info if available
                        ui_info = ui_params.get(param_name)

                        # Create parameter info
                        param_info = ParameterInfo(
                            name=param_name,
                            param_type=param_type,
                            default_value=extract_default_value(param),
                            constructor_default=constructor_defaults.get(param_name),
                            min_value=ui_info.min_value if ui_info else None,
                            max_value=ui_info.max_value if ui_info else None,
                            ui_default=ui_info.ui_default if ui_info else None,
                        )

                        # Get the best default value
                        parameters[param_name] = get_noticeable_default(param_info)
                        break

        return parameters
    except Exception as e:
        logger.warning(f"Failed to parse {filter_file} with libclang: {e}")
        # Fall back to regex-based parsing
        return parse_filter_parameters(filter_file, filter_id, special_values)


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
        ui_file = plugin_dir / "filterwidget.ui"

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

        # Try to parse with libclang first, fall back to regex if needed
        try:
            parameters = parse_filter_parameters_with_clang(
                filter_file, filter_id, SPECIAL_FILTER_VALUES, ui_file
            )
            for param_name, param_value in parameters.items():
                config.set("imageFilter1", param_name, param_value)

            # Try to extract plugin info with libclang
            plugin_data = extract_plugin_data_with_clang(main_cpp_path)
        except Exception as e:
            logger.warning(f"Error using libclang, falling back to regex: {e}")
            parameters = parse_filter_parameters(
                filter_file, filter_id, SPECIAL_FILTER_VALUES
            )
            for param_name, param_value in parameters.items():
                config.set("imageFilter1", param_name, param_value)
            plugin_data = extract_plugin_data(main_cpp_path)

        # Add plugin info
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

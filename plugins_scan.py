#!/usr/bin/env -S uv run
# /// script
# dependencies = ["clang", "pyyaml", "loguru", "pydantic"]
# ///

"""
Scanner for Image Filter Plugin configurations.
Extracts metadata from C++ source files and generates YAML configuration files.
"""

import json
import os
import re
import xml.etree.ElementTree as ET
from enum import Enum
from pathlib import Path
from typing import Any

import clang.cindex
import yaml
from clang.cindex import Config, Index
from loguru import logger
from plugins_utils import (FilterInfo, PluginInfo, PropertyDict, PropertyInfo,
                           PropertyType, UIInfo, sort_dict_recursive)

# Type aliases for better readability
PropertyDict = dict[str, "PropertyInfo"]
PluginInfo = dict[str, str]
UIInfo = dict[str, dict[str, float]]


def setup_libclang() -> None:
    """Configure libclang based on the system path."""
    # Try using brew prefix first
    try:
        import subprocess

        brew_llvm = subprocess.check_output(
            ["brew", "--prefix", "llvm"], text=True
        ).strip()
        brew_path = Path(brew_llvm) / "lib" / "libclang.dylib"
        if brew_path.exists():
            Config.set_library_file(str(brew_path))
            return
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass

    # Try using locate as fallback
    try:
        import subprocess

        paths = subprocess.check_output(
            ["locate", "libclang.dylib"], text=True
        ).splitlines()
        for path in paths:
            if os.path.exists(path):
                Config.set_library_file(path)
                return
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass

    logger.warning("Could not find libclang.dylib")


class CursorKinds:
    """Clang cursor kind values."""

    CALL_EXPR = 103
    INTEGER_LITERAL = 106
    FLOATING_LITERAL = 107
    STRING_LITERAL = 109
    CXX_BOOL_LITERAL_EXPR = 129
    CXX_METHOD = 21
    PARM_DECL = 10
    CONSTRUCTOR = 24


class EnumEncoder(json.JSONEncoder):
    """JSON encoder that handles Enum values."""

    def default(self, o: Any) -> Any:
        if isinstance(o, Enum):
            return o.value
        return super().default(o)


def find_string_literal(node: clang.cindex.Cursor) -> str | None:
    """Extract string literal from a clang cursor node."""
    if node.kind.value == CursorKinds.STRING_LITERAL:
        tokens = list(node.get_tokens())
        if tokens:
            return tokens[0].spelling.strip('"')

    for child in node.get_children():
        if result := find_string_literal(child):
            return result
    return None


def extract_plugin_info_clang(main_cpp_path: Path) -> PluginInfo:
    """Extract plugin info using libclang."""
    data: PluginInfo = {}
    try:
        index = Index.create()
        tu = index.parse(str(main_cpp_path), args=["-x", "c++", "-std=c++17"])

        for node in tu.cursor.walk_preorder():
            if node.kind.value == CursorKinds.CALL_EXPR and node.spelling == "insert":
                args = list(node.get_arguments())
                if len(args) < 2:
                    continue

                key = find_string_literal(args[0])
                if not key or key not in ["name", "description"]:
                    continue

                value = find_string_literal(args[1])
                if value:
                    data[key] = value

        return data
    except Exception as e:
        logger.warning(f"Failed to parse {main_cpp_path} with libclang: {e}")
        return {}


def extract_plugin_info_regex(main_cpp_path: Path) -> PluginInfo:
    """Extract plugin info using regex as fallback."""
    data: PluginInfo = {}
    try:
        content = main_cpp_path.read_text()
        patterns = {
            "name": r'info\.insert\("name",\s*QObject::tr\("([^"]+)"\)\);',
            "description": r'info\.insert\("description",\s*QObject::tr\("([^"]+)"\)\);',
        }

        for key, pattern in patterns.items():
            if match := re.search(pattern, content):
                data[key] = match.group(1)

        return data
    except Exception as e:
        logger.error(f"Failed to extract plugin info using regex: {e}")
        return {}


def extract_plugin_info(main_cpp_path: Path) -> PluginInfo:
    """Extract plugin name and description from main.cpp."""
    data = extract_plugin_info_clang(main_cpp_path)
    if not data:
        data = extract_plugin_info_regex(main_cpp_path)
    return data


def extract_ui_info(ui_file: Path) -> UIInfo:
    """Extract UI widget information from .ui file."""
    ui_info: UIInfo = {}

    if not ui_file.exists():
        return ui_info

    try:
        tree = ET.parse(ui_file)
        root = tree.getroot()

        for widget in root.findall(".//widget"):
            widget_name = widget.get("name", "")
            if not (
                widget_name.lower().startswith("mspin")
                or widget_name.lower().startswith("mslider")
            ):
                continue

            param_name = (
                widget_name[5:]
                if widget_name.lower().startswith("mspin")
                else widget_name[7:]
            ).lower()

            if param_name not in ui_info:
                ui_info[param_name] = {}

            for prop in widget.findall("property"):
                name_attr = prop.get("name")
                if name_attr and name_attr in ["minimum", "maximum", "value"]:
                    number_elem = prop.find("number")
                    if number_elem is not None and number_elem.text:
                        ui_info[param_name][name_attr] = float(number_elem.text)

        return ui_info
    except Exception as e:
        logger.warning(f"Failed to parse UI file {ui_file}: {e}")
        return {}


def create_property_info(
    param_name: str, param_type: str, ui_data: dict[str, float]
) -> PropertyInfo:
    """Create PropertyInfo object based on parameter type and UI data."""
    type_mapping = {
        "bool": (
            PropertyType.BOOL,
            False,
            True,
            0,
            1,
            "Toggle between true/false states",
        ),
        "int": (PropertyType.INT, 0, 75, 0, 100, "Integer value"),
        "double": (PropertyType.DOUBLE, 0.0, 0.75, 0.0, 100.0, "Floating point value"),
        "Color": (
            PropertyType.COLOR,
            "#000000",
            "#FF0000",
            None,
            None,
            "Color in hex format (#RRGGBB)",
        ),
        "enum": (PropertyType.ENUM, "0", "1", None, None, "Enumerated value"),
        "string": (PropertyType.STRING, "", "normal", None, None, "Text value"),
    }

    base_type = next((k for k in type_mapping if k in param_type), "string")
    prop_type, default_val, interesting_val, min_val, max_val, comment = type_mapping[
        base_type
    ]

    if base_type in ["int", "double"]:
        default_val = ui_data.get("value", default_val)
        min_val = ui_data.get("minimum", min_val)
        max_val = ui_data.get("maximum", max_val)

        # Ensure values match the type
        if base_type == "int":
            default_val = int(default_val)
            min_val = int(min_val) if min_val is not None else None
            max_val = int(max_val) if max_val is not None else None
            if min_val is not None and max_val is not None:
                comment = f"Integer value between {min_val} and {max_val}"
        else:
            default_val = float(default_val)
            min_val = float(min_val) if min_val is not None else None
            max_val = float(max_val) if max_val is not None else None
            if min_val is not None and max_val is not None:
                comment = (
                    f"Floating point value between {min_val:.1f} and {max_val:.1f}"
                )

        if min_val is not None and max_val is not None:
            interesting_val = min_val + (max_val - min_val) * 0.75
            if base_type == "int":
                interesting_val = int(interesting_val)
            else:
                interesting_val = round(float(interesting_val), 2)

    # Add enum-specific comment if it's an enum type
    if "enum" in param_type.lower():
        enum_match = re.search(r"enum\s*(?:\w+)?\s*\{([^}]+)\}", param_type)
        if enum_match:
            enum_values = [v.strip() for v in enum_match.group(1).split(",")]
            comment = f"Possible values: {', '.join(enum_values)}"

    return PropertyInfo(
        name=param_name,
        type=prop_type,
        default_value=default_val,
        interesting_value=interesting_val,
        description="",
        comment=comment,
        min_value=min_val,
        max_value=max_val,
    )


def extract_properties_from_header(
    h_file: Path, existing_props: PropertyDict
) -> PropertyDict:
    """Extract additional properties from the header file."""
    if not h_file.exists():
        return existing_props

    try:
        content = h_file.read_text()
        member_pattern = re.compile(r"\s*(bool|int|double)\s+m([A-Z]\w+)\s*;")

        for match in member_pattern.finditer(content):
            var_type, var_name = match.groups()
            prop_name = var_name.lower()

            if prop_name not in existing_props:
                prop_info = create_property_info(prop_name, var_type, {})
                existing_props[prop_name] = prop_info

        return existing_props
    except Exception as e:
        logger.warning(f"Failed to parse header file {h_file}: {e}")
        return existing_props


def extract_property_info(
    filter_file: Path, ui_file: Path | None = None
) -> PropertyDict:
    """Extract property information from filter files and UI files."""
    properties: PropertyDict = {}

    try:
        index = Index.create()
        tu = index.parse(str(filter_file), args=["-x", "c++", "-std=c++17"])
        ui_info = extract_ui_info(ui_file) if ui_file else {}

        for node in tu.cursor.walk_preorder():
            if node.kind.value == CursorKinds.CXX_METHOD and node.spelling.startswith(
                "set"
            ):
                param_name = node.spelling[3:].lower()

                for param in node.get_arguments():
                    if param.kind.value == CursorKinds.PARM_DECL:
                        param_type = param.type.spelling
                        ui_data = ui_info.get(param_name, {})
                        prop_info = create_property_info(
                            param_name, param_type, ui_data
                        )
                        properties[param_name] = prop_info

        # Add properties from header file
        h_file = filter_file.parent / "filter.h"
        properties = extract_properties_from_header(h_file, properties)

        return properties
    except Exception as e:
        logger.warning(f"Failed to extract property info: {e}")
        return properties


def is_valid_plugin_dir(plugin_dir: Path) -> bool:
    """Check if directory is a valid image filter plugin directory."""
    return (
        plugin_dir.is_dir()
        and plugin_dir.name.startswith("imagefilter_")
        and (plugin_dir / "filter.cpp").is_file()
        and (plugin_dir / "main.cpp").is_file()
    )


def get_plugin_paths(plugin_dir: Path) -> tuple[Path, Path, Path, Path]:
    """Get standard paths for a plugin directory."""
    return (
        plugin_dir / "filter.cpp",
        plugin_dir / "main.cpp",
        plugin_dir / "filterwidget.ui",
        plugin_dir / "filter.yaml",
    )


def create_filter_info(
    plugin_dir: Path, plugin_info: PluginInfo, properties: PropertyDict
) -> FilterInfo:
    """Create FilterInfo object for a plugin."""
    filter_id = plugin_dir.name.replace("imagefilter_", "")
    sorted_properties = {k: properties[k] for k in sorted(properties)}

    # Create example dictionary with interesting values
    example = {name: prop.interesting_value for name, prop in sorted_properties.items()}

    return FilterInfo(
        id=f"ibp.imagefilter.{filter_id}",
        name=plugin_info.get("name", filter_id),
        description=plugin_info.get(
            "description", f"Image filter plugin for {filter_id}"
        ),
        example=example,
        properties=sorted_properties,
    )


def save_yaml_file(yaml_file: Path, filter_info: FilterInfo) -> None:
    """Save FilterInfo to YAML file with all keys sorted alphabetically."""
    try:
        filter_dict = filter_info.model_dump(exclude_none=True)
        sorted_data = sort_dict_recursive(filter_dict)

        with yaml_file.open("w", encoding="utf-8") as f:
            yaml.safe_dump(
                sorted_data,
                f,
                default_flow_style=False,
                sort_keys=False,
                allow_unicode=True,
                encoding="utf-8",
            )
    except Exception as e:
        logger.error(f"Failed to save YAML file {yaml_file}: {e}")
        raise


def process_plugin_dir(plugin_dir: Path, base_dir: Path) -> Path | None:
    """Process a single plugin directory."""
    if not is_valid_plugin_dir(plugin_dir):
        return None

    try:
        filter_file, main_cpp, ui_file, yaml_file = get_plugin_paths(plugin_dir)
        plugin_info = extract_plugin_info(main_cpp)
        properties = extract_property_info(filter_file, ui_file)
        filter_info = create_filter_info(plugin_dir, plugin_info, properties)
        save_yaml_file(yaml_file, filter_info)

        return yaml_file.relative_to(base_dir)
    except Exception as e:
        logger.error(f"Failed to process {plugin_dir.name}: {e}")
        return None


def create_filter_yaml_files(base_dir: Path) -> list[Path]:
    """Create filter.yaml files for each image filter plugin."""
    plugins_dir = base_dir / "src" / "plugins"
    generated_files: list[Path] = []

    for plugin_dir in plugins_dir.iterdir():
        if yaml_path := process_plugin_dir(plugin_dir, base_dir):
            generated_files.append(yaml_path)

    return generated_files


def main() -> None:
    """Generate YAML files for all plugins and print their paths."""
    setup_libclang()
    base_dir = Path.cwd()
    yaml_files = create_filter_yaml_files(base_dir)
    print(" ".join(f'"{path}"' for path in yaml_files))


if __name__ == "__main__":
    main()

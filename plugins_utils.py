#!/usr/bin/env -S uv run
# /// script
# dependencies = ["pyyaml", "pydantic"]
# ///

"""
Common utilities for Image Filter Plugin processing.
"""

import json
from enum import Enum
from typing import Any

from pydantic import BaseModel

# Type aliases for better readability
PropertyDict = dict[str, "PropertyInfo"]
PluginInfo = dict[str, str]
UIInfo = dict[str, dict[str, float]]


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


class FilterInfo(BaseModel):
    """Complete information about a filter plugin."""

    id: str
    name: str
    description: str
    example: dict[str, int | float | str]
    properties: PropertyDict


def sort_dict_recursive(obj: Any) -> Any:
    """Recursively sort dictionary keys alphabetically."""
    if isinstance(obj, dict):
        return {k: sort_dict_recursive(obj[k]) for k in sorted(obj.keys())}
    if isinstance(obj, list):
        return [sort_dict_recursive(e) for e in obj]
    if isinstance(obj, Enum):
        return obj.value
    return obj

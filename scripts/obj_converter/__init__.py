"""
OBJ to custom C++ data generator.
"""
from .config import OUTPUT_CONFIGURATIONS
from .parser import read_obj
from .converter import convert_to_custom_format
# Re-export file I/O helpers
from .fileio import (
    ensure_dir,
    iter_obj_files,
    read_text,
    write_text,
    mount_colab_if_requested,
)
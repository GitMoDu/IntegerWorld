from typing import List, Dict, Any

# Configurations:
# - All output emits face normals and vertex normals.
# - Variants differ by centering and triangle winding (CW vs CCW "inverted").

OUTPUT_CONFIGURATIONS: List[Dict[str, Any]] = [
    {
        "name": "_raw",
        "center_vertices": False,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
        "emit_uv": True,
        "emit_uv_mips": True,
        "assume_ccw_winding": False,  # default CW
    },
    {
        "name": "_centered",
        "center_vertices": True,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
        "emit_uv": True,
        "emit_uv_mips": True,
        "assume_ccw_winding": False,  # default CW
    },
    {
        "name": "_raw_inverted",
        "center_vertices": False,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
        "emit_uv": True,
        "emit_uv_mips": True,
        "assume_ccw_winding": True,  # inverted CCW
    },
    {
        "name": "_centered_inverted",
        "center_vertices": True,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
        "emit_uv": True,
        "emit_uv_mips": True,
        "assume_ccw_winding": True,  # inverted CCW
    },
]
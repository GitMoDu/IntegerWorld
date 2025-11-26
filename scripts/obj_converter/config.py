from typing import List, Dict, Any

# Configurations:
# - All output emits CW indices and face normals.
# - Variants differ only by whether vertices are centered.

OUTPUT_CONFIGURATIONS: List[Dict[str, Any]] = [
    {
        "name": "_raw",
        "center_vertices": False,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
        "emit_uv": True,
        "emit_uv_mips": True,
    },
    {
        "name": "_centered",
        "center_vertices": True,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
        "emit_uv": True,
        "emit_uv_mips": True,
    },
]
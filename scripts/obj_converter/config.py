from typing import List, Dict, Any

# Updated configurations: removed obsolete CCW/CW and winding normalization flags.
# All output is now forced CW inside converter.py, so variants only differ by
# centering and whether face normals are emitted.
# Fields supported by converter:
#   center_vertices
#   emit_vertex_normals
#   emit_face_normals
#   emit_uv
#   emit_uv_mips
# (UV related feature flags like uv_v_flip, uv_wrap_mode, uv_force_pow2, etc.
# use converter defaults unless exposed here.)

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
    {
        "name": "_raw_facenorm",
        "center_vertices": False,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
        "emit_uv": True,
        "emit_uv_mips": True,
    },
    {
        "name": "_centered_facenorm",
        "center_vertices": True,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
        "emit_uv": True,
        "emit_uv_mips": True,
    },
]
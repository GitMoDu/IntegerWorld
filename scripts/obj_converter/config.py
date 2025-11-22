from typing import List, Dict, Any

# Output configurations replicated from the original script.
OUTPUT_CONFIGURATIONS: List[Dict[str, Any]] = [
    {
        "name": "_raw_CCW",
        "center_vertices": False,
        "apply_winding_normalization": True,
        "invert_winding_logic": False,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
    },
    {
        "name": "_centered_CCW",
        "center_vertices": True,
        "apply_winding_normalization": True,
        "invert_winding_logic": False,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
    },
    {
        "name": "_inverted_CW",
        "center_vertices": True,
        "apply_winding_normalization": True,
        "invert_winding_logic": True,
        "emit_vertex_normals": True,
        "emit_face_normals": True,
    },
    {
        "name": "_raw_no_norm_CCW",
        "center_vertices": False,
        "apply_winding_normalization": False,
        "invert_winding_logic": False,
        "emit_vertex_normals": True,
        "emit_face_normals": False,
    },
    {
        "name": "_raw_no_norm_CW",
        "center_vertices": False,
        "apply_winding_normalization": False,
        "invert_winding_logic": True,
        "emit_vertex_normals": True,
        "emit_face_normals": False,
    },
]
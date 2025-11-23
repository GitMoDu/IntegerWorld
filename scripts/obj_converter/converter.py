from typing import List, Tuple, Optional, Dict
import numpy as np

Vertex = Tuple[float, float, float]
Normal = Tuple[float, float, float]
IndexTriple = Tuple[int, Optional[int], Optional[int]]
Face = Tuple[IndexTriple, ...]


def convert_to_custom_format(
    vertices: List[Vertex],
    normals: List[Normal],
    faces_with_materials: List[Tuple[Face, Optional[str]]],
    file_name: str,
    *,
    center_vertices: bool = False,
    apply_winding_normalization: bool = True,
    invert_winding_logic: bool = False,
    emit_vertex_normals: bool = False,
    emit_face_normals: bool = False,
    use_macro_for_normal_scale: bool = True,  # if False will emit raw 8192 number
) -> str:
    """
    Converts parsed OBJ data to the custom C++ header-like text format.
    Optional vertex / face normals are normalized and scaled so vector magnitude == VERTEX16_UNIT (8192).
    Each component = round(unit_component * VERTEX16_UNIT). Degenerate normals fall back to {0,0,VERTEX16_UNIT}.
    """
    vertex_unit = 128  # vertex position scale
    NORMAL_SCALE = 8192  # matches VERTEX16_UNIT
    macro_name = "VERTEX16_UNIT"

    def format_component(value: int) -> str:
        # Replace exact +/- NORMAL_SCALE with +/- macro for readability if requested
        if use_macro_for_normal_scale:
            if value == NORMAL_SCALE:
                return macro_name
            if value == -NORMAL_SCALE:
                return f"-{macro_name}"
        return str(value)

    def normalize_and_scale(vec: np.ndarray) -> Tuple[int, int, int]:
        mag = float(np.linalg.norm(vec))
        if mag > 1e-6:
            unit = vec / mag
            x = int(round(unit[0] * NORMAL_SCALE))
            y = int(round(unit[1] * NORMAL_SCALE))
            z = int(round(unit[2] * NORMAL_SCALE))
            # Clamp to avoid overflow due to rounding
            x = max(-NORMAL_SCALE, min(NORMAL_SCALE, x))
            y = max(-NORMAL_SCALE, min(NORMAL_SCALE, y))
            z = max(-NORMAL_SCALE, min(NORMAL_SCALE, z))
            return x, y, z
        # Degenerate -> default +Z
        return 0, 0, NORMAL_SCALE

    output_lines: List[str] = []
    output_lines.append(f"namespace {file_name}\n{{")

    # Scaling constants
    output_lines.append("    static constexpr int16_t UpSize = 1;\n")
    output_lines.append("    static constexpr int16_t DownSize = 1;\n")

    # Optional: center vertices
    if center_vertices and vertices:
        vertices_np = np.array(vertices, dtype=float)
        center = np.mean(vertices_np, axis=0)
        processed_vertices = vertices_np - center
        print(f"Centering vertices around {center}")
    else:
        processed_vertices = np.array(vertices, dtype=float)

    # Vertices array
    output_lines.append("    static constexpr vertex16_t Vertices[] PROGMEM\n    {")
    for v in processed_vertices:
        output_lines.append(
            f"        {{(UpSize*(int32_t)({round(v[0] * vertex_unit)}))/DownSize , "
            f"(UpSize*(int32_t)({round(v[1] * vertex_unit)}))/DownSize , "
            f"(UpSize*(int32_t)({round(v[2] * vertex_unit)}))/DownSize}},"
        )
    output_lines.append("    };\n")
    output_lines.append("    constexpr auto VertexCount = sizeof(Vertices) / sizeof(vertex16_t);\n")

    # Triangulate faces, preserve material
    triangles_with_materials: List[Tuple[Tuple[IndexTriple, IndexTriple, IndexTriple], Optional[str]]] = []
    unique_materials: Dict[Optional[str], int] = {}
    material_index_counter = 0

    for face, material in faces_with_materials:
        tris: List[Tuple[IndexTriple, IndexTriple, IndexTriple]] = []
        if len(face) >= 3:
            v1 = face[0]
            for i in range(1, len(face) - 1):
                tris.append((v1, face[i], face[i + 1]))

        for tri in tris:
            triangles_with_materials.append((tri, material))

        if material not in unique_materials:
            unique_materials[material] = material_index_counter
            material_index_counter += 1

    # Process winding and collect face normals
    output_lines.append("    static constexpr triangle_face_t Triangles[] PROGMEM\n    {")
    processed_triangles: List[Tuple[int, int, int]] = []
    reversed_count = 0
    total_triangles = len(triangles_with_materials)
    normals_used_count = 0
    calculated_normals_used_count = 0

    face_norm_vectors: List[np.ndarray] = []

    for tri, _ in triangles_with_materials:
        v1i, v2i, v3i = tri
        v1_idx, v2_idx, v3_idx = v1i[0], v2i[0], v3i[0]
        should_reverse = False
        face_normal = None

        if apply_winding_normalization:
            if (
                v1i[2] is not None
                and v2i[2] is not None
                and v3i[2] is not None
                and normals
            ):
                vn1 = np.array(normals[v1i[2]], dtype=float)
                vn2 = np.array(normals[v2i[2]], dtype=float)
                vn3 = np.array(normals[v3i[2]], dtype=float)
                face_normal = (vn1 + vn2 + vn3) / 3.0
                normals_used_count += 1
            else:
                a = processed_vertices[v1_idx]
                b = processed_vertices[v2_idx]
                c = processed_vertices[v3_idx]
                face_normal = np.cross(b - a, c - a)
                calculated_normals_used_count += 1

            norm_mag = np.linalg.norm(face_normal)
            normal_dir = face_normal / norm_mag if norm_mag > 1e-6 else np.array([0.0, 0.0, 0.0])
            # Default "front" toward +Z
            view_direction = np.array([0.0, 0.0, 1.0], dtype=float)
            dot_product = float(np.dot(normal_dir, view_direction))

            if invert_winding_logic:
                if dot_product > 0.0:
                    should_reverse = True
            else:
                if dot_product < 0.0:
                    should_reverse = True
        elif invert_winding_logic:
            should_reverse = True
            a = processed_vertices[v1_idx]
            b = processed_vertices[v2_idx]
            c = processed_vertices[v3_idx]
            face_normal = np.cross(b - a, c - a)

        if should_reverse:
            processed_triangles.append((v1_idx, v3_idx, v2_idx))
            if face_normal is not None:
                face_normal = -face_normal
            reversed_count += 1
        else:
            processed_triangles.append((v1_idx, v2_idx, v3_idx))

        if emit_face_normals:
            if face_normal is None:
                a = processed_vertices[v1_idx]
                b = processed_vertices[v2_idx]
                c = processed_vertices[v3_idx]
                face_normal = np.cross(b - a, c - a)
            face_norm_vectors.append(face_normal)

    for t in processed_triangles:
        output_lines.append(f"        {{ {t[0]}, {t[1]}, {t[2]} }},")

    output_lines.append("    };\n")
    output_lines.append("    constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);\n")

    # Group array (material indices), 16 per line
    output_lines.append("    static constexpr uint8_t Group[TriangleCount] PROGMEM\n    {")
    material_indices = [unique_materials.get(mat, 0) for _, mat in triangles_with_materials]
    for i in range(0, len(material_indices), 16):
        chunk = ", ".join(str(x) for x in material_indices[i : i + 16])
        output_lines.append(f"        {chunk},")
    output_lines.append("    };\n")

    # Vertex normals (synthetic: direction of vertex from origin) scaled to NORMAL_SCALE
    if emit_vertex_normals:
        output_lines.append("    static constexpr vertex16_t VertexNormals[] PROGMEM\n    {")
        for v in processed_vertices:
            x, y, z = normalize_and_scale(v)
            output_lines.append(
                f"        {{{format_component(x)}, {format_component(y)}, {format_component(z)}}},"
            )
        output_lines.append("    };\n")
        output_lines.append("    constexpr auto VertexNormalCount = sizeof(VertexNormals) / sizeof(vertex16_t);\n")

    # Face normals (per triangle)
    if emit_face_normals:
        output_lines.append("    static constexpr vertex16_t FaceNormals[] PROGMEM\n    {")
        for nvec in face_norm_vectors:
            x, y, z = normalize_and_scale(nvec)
            output_lines.append(
                f"        {{{format_component(x)}, {format_component(y)}, {format_component(z)}}},"
            )
        output_lines.append("    };\n")
        output_lines.append("    constexpr auto FaceNormalCount = sizeof(FaceNormals) / sizeof(vertex16_t);\n")

    output_lines.append("}\n")

    if apply_winding_normalization:
        normalization_type = "Inverted" if invert_winding_logic else "Default"
        print(
            f"Processed {file_name}: Applied {normalization_type} winding order normalization "
            f"(reversed {reversed_count}/{total_triangles}); normals explicit {normals_used_count}, calculated {calculated_normals_used_count}."
        )
    elif invert_winding_logic:
        print(
            f"Processed {file_name}: Applied simple winding inversion "
            f"(reversed {reversed_count}/{total_triangles})."
        )
    else:
        print(f"Processed {file_name}: Winding normalization skipped.")

    if emit_vertex_normals:
        print(f"  Emitted {len(processed_vertices)} vertex normals (scale {NORMAL_SCALE}).")
    if emit_face_normals:
        print(f"  Emitted {len(face_norm_vectors)} face normals (scale {NORMAL_SCALE}).")

    return "\n".join(output_lines)
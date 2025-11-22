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
) -> str:
    """
    Converts parsed OBJ data to the custom C++ header-like text format.
    """
    vertex_unit = 128  # scale unit for vertices

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

    # Process winding and emit triangles
    output_lines.append("    static constexpr triangle_face_t Triangles[] PROGMEM\n    {")
    processed_triangles: List[Tuple[int, int, int]] = []
    reversed_count = 0
    total_triangles = len(triangles_with_materials)
    normals_used_count = 0
    calculated_normals_used_count = 0

    for tri, _ in triangles_with_materials:
        v1i, v2i, v3i = tri
        v1_idx, v2_idx, v3_idx = v1i[0], v2i[0], v3i[0]

        should_reverse = False

        if apply_winding_normalization:
            face_normal = None
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
                v1 = processed_vertices[v1_idx]
                v2 = processed_vertices[v2_idx]
                v3 = processed_vertices[v3_idx]
                vec1 = v2 - v1
                vec2 = v3 - v1
                face_normal = np.cross(vec1, vec2)
                calculated_normals_used_count += 1

            norm_mag = np.linalg.norm(face_normal)
            if norm_mag > 1e-6:
                normalized_face_normal = face_normal / norm_mag
            else:
                normalized_face_normal = np.array([0.0, 0.0, 0.0], dtype=float)

            view_direction = np.array([0.0, 0.0, 1.0], dtype=float)
            dot_product = float(np.dot(normalized_face_normal, view_direction))

            if invert_winding_logic:
                if dot_product > 0.0:
                    should_reverse = True
            else:
                if dot_product < 0.0:
                    should_reverse = True
        elif invert_winding_logic:
            should_reverse = True

        if should_reverse:
            processed_triangles.append((v1_idx, v3_idx, v2_idx))
            reversed_count += 1
        else:
            processed_triangles.append((v1_idx, v2_idx, v3_idx))

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

    output_lines.append("}\n")

    if apply_winding_normalization:
        normalization_type = "Inverted" if invert_winding_logic else "Default"
        print(
            f"Processed {file_name}: Applied {normalization_type} winding order normalization "
            f"(reversed for {reversed_count}/{total_triangles} triangles). "
            f"Used explicit normals for {normals_used_count} and calculated normals for {calculated_normals_used_count}."
        )
    elif invert_winding_logic:
        print(
            f"Processed {file_name}: Applied simple winding order inversion "
            f"(reversed for {reversed_count}/{total_triangles} triangles)."
        )
    else:
        print(f"Processed {file_name}: Winding order normalization and inversion skipped.")

    return "\n".join(output_lines)
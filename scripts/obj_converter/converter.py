from typing import List, Tuple, Optional, Dict, NamedTuple
import numpy as np

Vertex = Tuple[float, float, float]
Normal = Tuple[float, float, float]
IndexTriple = Tuple[int, Optional[int], Optional[int]]
Face = Tuple[IndexTriple, ...]


class ConversionResult(NamedTuple):
    text: str
    triangle_count: int
    reversed_count: int
    explicit_normals_used: int
    calculated_normals_used: int


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
    return _convert(vertices, normals, faces_with_materials, file_name,
                    center_vertices=center_vertices,
                    apply_winding_normalization=apply_winding_normalization,
                    invert_winding_logic=invert_winding_logic).text


def _convert(
    vertices: List[Vertex],
    normals: List[Normal],
    faces_with_materials: List[Tuple[Face, Optional[str]]],
    file_name: str,
    *,
    center_vertices: bool,
    apply_winding_normalization: bool,
    invert_winding_logic: bool,
) -> ConversionResult:
    vertex_unit = 128
    output_lines: List[str] = [f"namespace {file_name}\n{{"]
    output_lines.append("    static constexpr int16_t UpSize = 1;\n")
    output_lines.append("    static constexpr int16_t DownSize = 1;\n")

    if center_vertices and vertices:
        vertices_np = np.array(vertices, dtype=float)
        center = np.mean(vertices_np, axis=0)
        processed_vertices = vertices_np - center
        print(f"Centering vertices around {center}")
    else:
        processed_vertices = np.array(vertices, dtype=float)

    output_lines.append("    static constexpr vertex16_t Vertices[] PROGMEM\n    {")
    for v in processed_vertices:
        output_lines.append(
            f"        {{(UpSize*(int32_t)({round(v[0] * vertex_unit)}))/DownSize , "
            f"(UpSize*(int32_t)({round(v[1] * vertex_unit)}))/DownSize , "
            f"(UpSize*(int32_t)({round(v[2] * vertex_unit)}))/DownSize}},"
        )
    output_lines.append("    };\n")
    output_lines.append("    constexpr auto VertexCount = sizeof(Vertices) / sizeof(vertex16_t);\n")

    triangles_with_materials: List[Tuple[Tuple[IndexTriple, IndexTriple, IndexTriple], str]] = []
    unique_materials: Dict[str, int] = {}
    material_index_counter = 0

    for face, material in faces_with_materials:
        material_name = material or "DefaultMaterial"
        if len(face) >= 3:
            v1 = face[0]
            for i in range(1, len(face) - 1):
                tri = (v1, face[i], face[i + 1])
                triangles_with_materials.append((tri, material_name))
        if material_name not in unique_materials:
            unique_materials[material_name] = material_index_counter
            material_index_counter += 1

    output_lines.append("    static constexpr triangle_face_t Triangles[] PROGMEM\n    {")
    processed_triangles: List[Tuple[int, int, int]] = []
    reversed_count = 0
    normals_used_count = 0
    calculated_normals_used_count = 0

    for tri, _ in triangles_with_materials:
        v1i, v2i, v3i = tri
        v1_idx, v2_idx, v3_idx = v1i[0], v2i[0], v3i[0]
        should_reverse = False

        if apply_winding_normalization:
            if all(v[2] is not None for v in tri) and normals:
                vn = [np.array(normals[v[2]], dtype=float) for v in tri]
                face_normal = sum(vn) / 3.0
                normals_used_count += 1
            else:
                a, b, c = processed_vertices[v1_idx], processed_vertices[v2_idx], processed_vertices[v3_idx]
                face_normal = np.cross(b - a, c - a)
                calculated_normals_used_count += 1

            norm_mag = np.linalg.norm(face_normal)
            normal_dir = face_normal / norm_mag if norm_mag > 1e-6 else np.array([0.0, 0.0, 0.0])
            dot_product = float(np.dot(normal_dir, np.array([0.0, 0.0, 1.0])))

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

    output_lines.append("    static constexpr uint8_t Group[TriangleCount] PROGMEM\n    {")
    material_indices = [unique_materials[m] for _, m in triangles_with_materials]
    for i in range(0, len(material_indices), 16):
        output_lines.append("        " + ", ".join(str(x) for x in material_indices[i:i + 16]) + ",")
    output_lines.append("    };\n")
    output_lines.append("}\n")

    total_triangles = len(processed_triangles)

    return ConversionResult(
        text="\n".join(output_lines),
        triangle_count=total_triangles,
        reversed_count=reversed_count,
        explicit_normals_used=normals_used_count,
        calculated_normals_used=calculated_normals_used_count,
    )
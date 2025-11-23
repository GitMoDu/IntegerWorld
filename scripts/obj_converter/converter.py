from typing import List, Tuple, Optional, Dict
import numpy as np
import math

Vertex = Tuple[float, float, float]
TexCoord = Tuple[float, ...]
Normal = Tuple[float, float, float]
IndexTriple = Tuple[int, Optional[int], Optional[int]]
Face = Tuple[IndexTriple, ...]


def convert_to_custom_format(
    vertices: List[Vertex],
    texcoords: List[TexCoord],
    normals: List[Normal],
    faces_with_materials: List[Tuple[Face, Optional[str]]],
    file_name: str,
    *,
    center_vertices: bool = False,
    apply_winding_normalization: bool = True,
    invert_winding_logic: bool = False,
    emit_vertex_normals: bool = False,
    emit_face_normals: bool = False,
    use_macro_for_normal_scale: bool = True,
    emit_uv: bool = True,
    emit_uv_mips: bool = True,
    texture_width: Optional[int] = None,
    texture_height: Optional[int] = None,
    uv_force_pow2: bool = True,
) -> str:
    """
    Converts parsed OBJ data to the custom C++ header-like text format.
    - Normals scaled to VERTEX16_UNIT (8192).
    - UVs: emitted ONLY if texture_width/texture_height are provided (from an image).
      When provided:
        * Dimensions optionally rounded up to power-of-two if uv_force_pow2=True.
        * If all UVs are within [0,1], scale directly to pixel grid.
        * Otherwise normalize over min..max before quantization.
      Arrays:
        static constexpr coordinate_t UVs{W}x{H}[UvCount] PROGMEM
        Optional mips UVs{w}x{h}_L{level} for each halved dimension until 1x1.
      Note: UvMipLevelCount includes the master level (level 0).
    """
    vertex_unit = 128
    NORMAL_SCALE = 8192
    macro_name = "VERTEX16_UNIT"

    def format_component(value: int) -> str:
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
            x = max(-NORMAL_SCALE, min(NORMAL_SCALE, x))
            y = max(-NORMAL_SCALE, min(NORMAL_SCALE, y))
            z = max(-NORMAL_SCALE, min(NORMAL_SCALE, z))
            return x, y, z
        return 0, 0, NORMAL_SCALE

    output_lines: List[str] = []
    output_lines.append(f"namespace {file_name}\n{{")
    output_lines.append("    static constexpr int16_t UpSize = 1;\n")
    output_lines.append("    static constexpr int16_t DownSize = 1;\n")

    # Optional centering
    if center_vertices and vertices:
        vertices_np = np.array(vertices, dtype=float)
        center = np.mean(vertices_np, axis=0)
        processed_vertices = vertices_np - center
        print(f"Centering vertices around {center}")
    else:
        processed_vertices = np.array(vertices, dtype=float)

    # Vertices
    output_lines.append("    static constexpr vertex16_t Vertices[] PROGMEM\n    {")
    for v in processed_vertices:
        output_lines.append(
            f"        {{(UpSize*(int32_t)({round(v[0] * vertex_unit)}))/DownSize , "
            f"(UpSize*(int32_t)({round(v[1] * vertex_unit)}))/DownSize , "
            f"(UpSize*(int32_t)({round(v[2] * vertex_unit)}))/DownSize}},"
        )
    output_lines.append("    };\n")
    output_lines.append("    constexpr auto VertexCount = sizeof(Vertices) / sizeof(vertex16_t);\n")

    # Triangulation
    triangles_with_materials: List[Tuple[Tuple[IndexTriple, IndexTriple, IndexTriple], Optional[str]]] = []
    unique_materials: Dict[Optional[str], int] = {}
    material_index_counter = 0
    for face, material in faces_with_materials:
        if len(face) >= 3:
            v1 = face[0]
            for i in range(1, len(face) - 1):
                tri = (v1, face[i], face[i + 1])
                triangles_with_materials.append((tri, material))
        if material not in unique_materials:
            unique_materials[material] = material_index_counter
            material_index_counter += 1

    # Triangles & normals
    output_lines.append("    static constexpr triangle_face_t Triangles[] PROGMEM\n    {")
    processed_triangles: List[Tuple[int, int, int]] = []
    reversed_count = 0
    normals_used_count = 0
    calculated_normals_used_count = 0
    face_norm_vectors: List[np.ndarray] = []
    total_tris = len(triangles_with_materials)

    for tri, _ in triangles_with_materials:
        v1i, v2i, v3i = tri
        v1_idx, v2_idx, v3_idx = v1i[0], v2i[0], v3i[0]
        should_reverse = False
        face_normal = None
        if apply_winding_normalization:
            if v1i[2] is not None and v2i[2] is not None and v3i[2] is not None and normals:
                vn1 = np.array(normals[v1i[2]], dtype=float)
                vn2 = np.array(normals[v2i[2]], dtype=float)
                vn3 = np.array(normals[v3i[2]], dtype=float)
                face_normal = (vn1 + vn2 + vn3) / 3.0
                normals_used_count += 1
            else:
                a = processed_vertices[v1_idx]; b = processed_vertices[v2_idx]; c = processed_vertices[v3_idx]
                face_normal = np.cross(b - a, c - a)
                calculated_normals_used_count += 1
            norm_mag = np.linalg.norm(face_normal)
            normal_dir = face_normal / norm_mag if norm_mag > 1e-6 else np.array([0.0, 0.0, 0.0])
            view_dir = np.array([0.0, 0.0, 1.0])
            dot = float(np.dot(normal_dir, view_dir))
            if invert_winding_logic:
                if dot > 0.0:
                    should_reverse = True
            else:
                if dot < 0.0:
                    should_reverse = True
        elif invert_winding_logic:
            should_reverse = True
            a = processed_vertices[v1_idx]; b = processed_vertices[v2_idx]; c = processed_vertices[v3_idx]
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
                a = processed_vertices[v1_idx]; b = processed_vertices[v2_idx]; c = processed_vertices[v3_idx]
                face_normal = np.cross(b - a, c - a)
            face_norm_vectors.append(face_normal)

    for a, b, c in processed_triangles:
        output_lines.append(f"        {{ {a}, {b}, {c} }},")
    output_lines.append("    };\n")
    output_lines.append("    constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);\n")

    # Group indices
    output_lines.append("    static constexpr uint8_t Group[TriangleCount] PROGMEM\n    {")
    material_indices = [unique_materials.get(mat, 0) for _, mat in triangles_with_materials]
    for i in range(0, len(material_indices), 16):
        output_lines.append("        " + ", ".join(str(x) for x in material_indices[i:i+16]) + ",")
    output_lines.append("    };\n")

    # Normals
    if emit_vertex_normals:
        output_lines.append("    static constexpr vertex16_t VertexNormals[] PROGMEM\n    {")
        for v in processed_vertices:
            x, y, z = normalize_and_scale(v)
            output_lines.append(f"        {{{format_component(x)}, {format_component(y)}, {format_component(z)}}},")
        output_lines.append("    };\n")
        output_lines.append("    constexpr auto VertexNormalCount = sizeof(VertexNormals) / sizeof(vertex16_t);\n")

    if emit_face_normals:
        output_lines.append("    static constexpr vertex16_t FaceNormals[] PROGMEM\n    {")
        for nvec in face_norm_vectors:
            x, y, z = normalize_and_scale(nvec)
            output_lines.append(f"        {{{format_component(x)}, {format_component(y)}, {format_component(z)}}},")
        output_lines.append("    };\n")
        output_lines.append("    constexpr auto FaceNormalCount = sizeof(FaceNormals) / sizeof(vertex16_t);\n")

    # UV emission (only if a texture was found)
    has_texture_dims = texture_width is not None and texture_height is not None
    if emit_uv and has_texture_dims:
        # Collect per-triangle vertex UV indices
        used_uv_indices: List[int] = []
        for tri, _ in triangles_with_materials:
            for idx_trip in tri:
                uv_index = idx_trip[1]
                used_uv_indices.append(uv_index if uv_index is not None else -1)

        # Fallback UV generation if none present: simple planar projection (XY)
        if not texcoords:
            texcoords = [(float(v[0]), float(v[1])) for v in processed_vertices]

        # Gather used u,v
        u_values: List[float] = []
        v_values: List[float] = []
        for uv_idx in used_uv_indices:
            if 0 <= uv_idx < len(texcoords):
                uv = texcoords[uv_idx]
                if len(uv) >= 2:
                    u_values.append(uv[0])
                    v_values.append(uv[1])

        if not u_values or not v_values:
            u_values = [0.0]
            v_values = [0.0]

        u_min, u_max = min(u_values), max(u_values)
        v_min, v_max = min(v_values), max(v_values)
        if abs(u_max - u_min) < 1e-9:
            u_max = u_min + 1.0
        if abs(v_max - v_min) < 1e-9:
            v_max = v_min + 1.0

        eps = 1e-6
        all_norm = all(-eps <= u <= 1.0 + eps for u in u_values) and all(-eps <= v <= 1.0 + eps for v in v_values)

        def next_pow2(n: int) -> int:
            return 1 << (n - 1).bit_length()

        base_w = max(1, int(texture_width))   # type: ignore[arg-type]
        base_h = max(1, int(texture_height))  # type: ignore[arg-type]
        if uv_force_pow2:
            base_w = next_pow2(base_w)
            base_h = next_pow2(base_h)
        width = base_w
        height = base_h
        direct_scale = all_norm

        output_lines.append(f"    static constexpr uint16_t UvMasterWidth = {width};\n")
        output_lines.append(f"    static constexpr uint16_t UvMasterHeight = {height};\n")
        output_lines.append("    static constexpr uint16_t UvCount = TriangleCount * 3;\n")

        # Precompute master integer UVs once
        master_uvs: List[Tuple[int, int]] = []
        for tri, _ in triangles_with_materials:
            for idx_trip in tri:
                uv_idx = idx_trip[1]
                if uv_idx is not None and 0 <= uv_idx < len(texcoords) and len(texcoords[uv_idx]) >= 2:
                    u_raw, v_raw = texcoords[uv_idx][0], texcoords[uv_idx][1]
                else:
                    vpos = processed_vertices[idx_trip[0]]
                    u_raw, v_raw = float(vpos[0]), float(vpos[1])

                if direct_scale:
                    u_norm = max(0.0, min(1.0, u_raw))
                    v_norm = max(0.0, min(1.0, v_raw))
                else:
                    u_norm = (u_raw - u_min) / (u_max - u_min)
                    v_norm = (v_raw - v_min) / (v_max - v_min)

                u_q = int(round(u_norm * (width - 1)))
                v_q = int(round(v_norm * (height - 1)))
                master_uvs.append((u_q, v_q))

        output_lines.append(f"    static constexpr coordinate_t UVs{width}x{height}[UvCount] PROGMEM\n    {{")
        for (u_q, v_q) in master_uvs:
            output_lines.append(f"        {{{u_q}, {v_q}}},")
        output_lines.append("    };\n")

        # Compute total level count (includes master)
        def level_count(w: int, h: int) -> int:
            cnt = 1
            while w > 1 or h > 1:
                w = max(1, w // 2)
                h = max(1, h // 2)
                cnt += 1
            return cnt

        if emit_uv_mips:
            mip_w, mip_h = width, height
            level = 1
            while mip_w > 1 or mip_h > 1:
                mip_w = max(1, mip_w // 2)
                mip_h = max(1, mip_h // 2)
                output_lines.append(f"    static constexpr coordinate_t UVs{mip_w}x{mip_h}_L{level}[UvCount] PROGMEM\n    {{")
                for (u_q_master, v_q_master) in master_uvs:
                    u_q = min(mip_w - 1, u_q_master >> level)
                    v_q = min(mip_h - 1, v_q_master >> level)
                    output_lines.append(f"        {{{u_q}, {v_q}}},")
                output_lines.append("    };\n")
                level += 1
            output_lines.append(f"    static constexpr uint8_t UvMipLevelCount = {level_count(width, height)};\n")
        else:
            output_lines.append("    static constexpr uint8_t UvMipLevelCount = 1;\n")

    output_lines.append("}\n")

    # Diagnostics
    if apply_winding_normalization:
        print(f"Processed {file_name}: winding normalized (reversed {reversed_count}/{total_tris}); normals explicit {normals_used_count}, calculated {calculated_normals_used_count}.")
    elif invert_winding_logic:
        print(f"Processed {file_name}: winding inverted (reversed {reversed_count}/{total_tris}).")
    else:
        print(f"Processed {file_name}: winding untouched.")
    if emit_vertex_normals:
        print(f"  Emitted {len(processed_vertices)} vertex normals.")
    if emit_face_normals:
        print(f"  Emitted {len(face_norm_vectors)} face normals.")
    # UV diagnostics
    if emit_uv:
        if has_texture_dims:
            # width/level only defined when UVs were emitted
            print(f"  UVs: emitted (master {width}x{height}, mip levels {'on' if emit_uv_mips else 'off'}).")
        else:
            print("  UVs: skipped (no texture found).")

    return "\n".join(output_lines)
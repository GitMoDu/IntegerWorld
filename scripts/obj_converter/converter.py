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
    uv_v_flip: bool = True,            # DEFAULT FLIP ENABLED
    uv_wrap_mode: str = "auto",        # 'auto' | 'clamp' | 'wrap'
) -> str:
    """
    Converts OBJ data to custom C++ header-like text format.
    UV handling:
      uv_v_flip (default True): flip V (v = 1 - v) after normalization/wrapping to match typical top-left texture origin.
      uv_wrap_mode:
        clamp: clamp UVs into [0,1]
        wrap:  wrap UVs via fractional part (supports negative values)
        auto:  wrap if any UV outside [0,1], otherwise clamp
    UVs emitted only when texture dimensions provided.
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

    triangles_with_materials: List[Tuple[Tuple[IndexTriple, IndexTriple, IndexTriple], Optional[str]]] = []
    unique_materials: Dict[Optional[str], int] = {}
    material_index_counter = 0
    for face, material in faces_with_materials:
        if len(face) >= 3:
            v1 = face[0]
            for i in range(1, len(face) - 1):
                triangles_with_materials.append(((v1, face[i], face[i + 1]), material))
        if material not in unique_materials:
            unique_materials[material] = material_index_counter
            material_index_counter += 1

    # Precompute model centroid for outward direction tests
    model_centroid = np.mean(processed_vertices, axis=0) if len(processed_vertices) else np.array([0.0, 0.0, 0.0])

    # Triangles & normals (updated winding logic)
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
        a = processed_vertices[v1_idx]
        b = processed_vertices[v2_idx]
        c = processed_vertices[v3_idx]

        # Base face normal
        face_normal = np.cross(b - a, c - a)
        area_mag = np.linalg.norm(face_normal)
        if area_mag > 1e-12:
            face_normal_dir = face_normal / area_mag
        else:
            face_normal_dir = np.array([0.0, 0.0, 0.0])

        # Try vertex normals if all available
        use_vertex_norms = (
            apply_winding_normalization
            and v1i[2] is not None
            and v2i[2] is not None
            and v3i[2] is not None
            and normals
        )
        avg_vertex_normal = None
        if use_vertex_norms:
            vn1 = np.array(normals[v1i[2]], dtype=float)
            vn2 = np.array(normals[v2i[2]], dtype=float)
            vn3 = np.array(normals[v3i[2]], dtype=float)
            avg_vertex_normal = (vn1 + vn2 + vn3) / 3.0
            vm = np.linalg.norm(avg_vertex_normal)
            if vm > 1e-12:
                avg_vertex_normal /= vm

        # Outward test
        tri_centroid = (a + b + c) / 3.0
        center_vec = tri_centroid - model_centroid
        center_len = np.linalg.norm(center_vec)
        if center_len > 1e-12:
            center_dir = center_vec / center_len
        else:
            center_dir = np.array([0.0, 0.0, 0.0])

        should_reverse = False
        if apply_winding_normalization:
            # Prefer vertex normals if present; ensure face normal aligns with outward direction
            if avg_vertex_normal is not None:
                if np.dot(face_normal_dir, avg_vertex_normal) < 0.0:
                    should_reverse = True
                normals_used_count += 1
            else:
                # Use centroid direction for outward orientation
                if np.dot(face_normal_dir, center_dir) < 0.0:
                    should_reverse = True
                calculated_normals_used_count += 1

        # Optional global inversion
        if invert_winding_logic:
            should_reverse = not should_reverse

        if should_reverse:
            processed_triangles.append((v1_idx, v3_idx, v2_idx))
            face_normal = -face_normal
            reversed_count += 1
        else:
            processed_triangles.append((v1_idx, v2_idx, v3_idx))

        if emit_face_normals:
            # Store (possibly flipped) face normal
            face_norm_vectors.append(face_normal)

    for a, b, c in processed_triangles:
        output_lines.append(f"        {{ {a}, {b}, {c} }},")
    output_lines.append("    };\n")
    output_lines.append("    constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);\n")

    output_lines.append("    static constexpr uint8_t Group[TriangleCount] PROGMEM\n    {")
    material_indices = [unique_materials.get(mat, 0) for _, mat in triangles_with_materials]
    for i in range(0, len(material_indices), 16):
        output_lines.append("        " + ", ".join(str(x) for x in material_indices[i:i+16]) + ",")
    output_lines.append("    };\n")

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

    has_texture_dims = texture_width is not None and texture_height is not None
    if emit_uv and has_texture_dims:
        used_uv_indices: List[int] = []
        for tri, _ in triangles_with_materials:
            for idx_trip in tri:
                used_uv_indices.append(idx_trip[1] if idx_trip[1] is not None else -1)

        if not texcoords:
            texcoords = [(float(v[0]), float(v[1])) for v in processed_vertices]

        u_values: List[float] = []
        v_values: List[float] = []
        for uv_idx in used_uv_indices:
            if 0 <= uv_idx < len(texcoords):
                uv = texcoords[uv_idx]
                if len(uv) >= 2:
                    u_values.append(uv[0]); v_values.append(uv[1])

        if not u_values or not v_values:
            u_values = [0.0]; v_values = [0.0]

        out_of_range = any(u < 0.0 or u > 1.0 for u in u_values) or any(v < 0.0 or v > 1.0 for v in v_values)
        eps = 1e-6
        all_norm = not out_of_range and all(-eps <= u <= 1.0 + eps for u in u_values) and all(-eps <= v <= 1.0 + eps for v in v_values)

        def wrap01(x: float) -> float:
            return ((x % 1.0) + 1.0) % 1.0

        def next_pow2(n: int) -> int:
            return 1 << (n - 1).bit_length()

        base_w = max(1, int(texture_width))   # type: ignore[arg-type]
        base_h = max(1, int(texture_height))  # type: ignore[arg-type]
        if uv_force_pow2:
            base_w = next_pow2(base_w)
            base_h = next_pow2(base_h)
        width, height = base_w, base_h

        if uv_wrap_mode == "auto":
            mode = "wrap" if out_of_range else "clamp"
        elif uv_wrap_mode in ("wrap", "clamp"):
            mode = uv_wrap_mode
        else:
            mode = "clamp"

        output_lines.append(f"    static constexpr uint16_t UvMasterWidth = {width};\n")
        output_lines.append(f"    static constexpr uint16_t UvMasterHeight = {height};\n")
        output_lines.append("    static constexpr uint16_t UvCount = TriangleCount * 3;\n")

        master_uvs: List[Tuple[int, int]] = []
        # Precompute min/max only once for range mode
        u_min, u_max = min(u_values), max(u_values)
        v_min, v_max = min(v_values), max(v_values)
        if abs(u_max - u_min) < 1e-9: u_max = u_min + 1.0
        if abs(v_max - v_min) < 1e-9: v_max = v_min + 1.0

        for tri, _ in triangles_with_materials:
            for idx_trip in tri:
                uv_idx = idx_trip[1]
                if uv_idx is not None and 0 <= uv_idx < len(texcoords) and len(texcoords[uv_idx]) >= 2:
                    u_raw, v_raw = texcoords[uv_idx][0], texcoords[uv_idx][1]
                else:
                    vpos = processed_vertices[idx_trip[0]]
                    u_raw, v_raw = float(vpos[0]), float(vpos[1])

                if all_norm:
                    if mode == "wrap":
                        u_norm = wrap01(u_raw); v_norm = wrap01(v_raw)
                    else:
                        u_norm = min(max(u_raw, 0.0), 1.0)
                        v_norm = min(max(v_raw, 0.0), 1.0)
                else:
                    if mode == "wrap":
                        u_norm = wrap01(u_raw)
                        v_norm = wrap01(v_raw)
                    else:
                        u_norm = (u_raw - u_min) / (u_max - u_min)
                        v_norm = (v_raw - v_min) / (v_max - v_min)

                if uv_v_flip:
                    v_norm = 1.0 - v_norm

                u_q = int(round(u_norm * (width - 1)))
                v_q = int(round(v_norm * (height - 1)))
                master_uvs.append((u_q, v_q))

        output_lines.append(f"    static constexpr coordinate_t UVs{width}x{height}[UvCount] PROGMEM\n    {{")
        for (u_q, v_q) in master_uvs:
            output_lines.append(f"        {{{u_q}, {v_q}}},")
        output_lines.append("    };\n")

        MIN_MIP_DIM = 8

        def level_count(w: int, h: int) -> int:
            cnt = 1
            while w > MIN_MIP_DIM or h > MIN_MIP_DIM:
                w_next = max(MIN_MIP_DIM, w // 2)
                h_next = max(MIN_MIP_DIM, h // 2)
                if w_next == w and h_next == h:
                    break
                w, h = w_next, h_next
                cnt += 1
            return cnt

        if emit_uv_mips:
            mip_w, mip_h = width, height
            level = 1
            while mip_w > MIN_MIP_DIM or mip_h > MIN_MIP_DIM:
                mip_w_next = max(MIN_MIP_DIM, mip_w // 2)
                mip_h_next = max(MIN_MIP_DIM, mip_h // 2)
                if mip_w_next == mip_w and mip_h_next == mip_h:
                    break
                mip_w, mip_h = mip_w_next, mip_h_next
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

    if emit_uv and has_texture_dims:
        resolved_mode = ('wrap' if (uv_wrap_mode == 'wrap' or (uv_wrap_mode == 'auto' and out_of_range)) else 'clamp')
        print(f"  UVs: mode={resolved_mode}, flipV={uv_v_flip}")

    return "\n".join(output_lines)
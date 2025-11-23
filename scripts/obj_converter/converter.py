from typing import List, Tuple, Optional, Dict
import numpy as np

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
    # Engine consumes clockwise (CW) mesh winding; we now always emit CW only.
    emit_vertex_normals: bool = True,
    emit_face_normals: bool = True,
    use_macro_for_normal_scale: bool = True,
    emit_uv: bool = True,
    emit_uv_mips: bool = True,
    texture_width: Optional[int] = None,
    texture_height: Optional[int] = None,
    uv_force_pow2: bool = True,
    uv_v_flip: bool = True,
    uv_wrap_mode: str = "auto",
) -> str:
    """
    Converts OBJ data to custom C++ header-like text format.

    Winding:
      OBJ convention is typically counter-clockwise (CCW) for front faces.
      The engine expects clockwise (CW) ordering; all triangulated faces are
      converted to CW (second/third vertex swapped) unconditionally.

    Normals:
      VertexNormals:
        Averaged from all OBJ-corner normals referencing each vertex index,
        with fallback to averaged geometric face normals if no explicit contributions.
        Quantization enforces integer vector magnitude == VERTEX16_UNIT (8192).
      FaceNormals:
        If OBJ contains vertex normals (n indices for every triangle corner) but no
        separate face normals, we "interpolate" a face normal by averaging the three
        referenced vertex normals (n1 + n2 + n3). This produces a smoothed face normal.
        If any corner lacks a normal index (or normals list empty) we fall back to the
        geometric cross product normal of the triangle (using CW ordering).
        Quantized to magnitude == VERTEX16_UNIT. Degenerate -> (0,0,VERTEX16_UNIT).

    UV handling:
      uv_v_flip (default True): flip V (v = 1 - v) after normalization/wrapping.
      uv_wrap_mode: clamp | wrap | auto
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

    def quantize_normal(unit: np.ndarray) -> Tuple[int, int, int]:
        """
        Quantize a floating unit vector so that the resulting integer vector
        has magnitude exactly NORMAL_SCALE (or as close as possible without exceeding).
        """
        # Initial scaled (float) components
        scaled = unit * NORMAL_SCALE
        xi = int(round(scaled[0]))
        yi = int(round(scaled[1]))
        zi = int(round(scaled[2]))

        # Clamp individual components (safety)
        xi = max(-NORMAL_SCALE, min(NORMAL_SCALE, xi))
        yi = max(-NORMAL_SCALE, min(NORMAL_SCALE, yi))
        zi = max(-NORMAL_SCALE, min(NORMAL_SCALE, zi))

        # Adjust magnitude to be <= NORMAL_SCALE and try to reach exactly NORMAL_SCALE.
        length_sq = xi * xi + yi * yi + zi * zi
        target_sq = NORMAL_SCALE * NORMAL_SCALE

        if length_sq == 0:
            return 0, 0, NORMAL_SCALE  # fallback

        # If overshoot or undershoot, re-scale using float factor, then round again.
        if length_sq != target_sq:
            length = float(np.sqrt(length_sq))
            scale = NORMAL_SCALE / length  # bring magnitude to EXACT NORMAL_SCALE
            x2 = int(round(xi * scale))
            y2 = int(round(yi * scale))
            z2 = int(round(zi * scale))

            # Final clamp
            x2 = max(-NORMAL_SCALE, min(NORMAL_SCALE, x2))
            y2 = max(-NORMAL_SCALE, min(NORMAL_SCALE, y2))
            z2 = max(-NORMAL_SCALE, min(NORMAL_SCALE, z2))

            # Recompute length; if still off, accept closest (rounding limitation).
            length_sq_2 = x2 * x2 + y2 * y2 + z2 * z2
            if length_sq_2 == 0:
                return 0, 0, NORMAL_SCALE
            # Prefer the rescaled version if it is closer to target.
            if abs(length_sq_2 - target_sq) <= abs(length_sq - target_sq):
                xi, yi, zi = x2, y2, z2

        return xi, yi, zi

    def normalize_float(vec: np.ndarray) -> np.ndarray:
        mag = float(np.linalg.norm(vec))
        if mag > 1e-9:
            return vec / mag
        return np.array([0.0, 0.0, 1.0], dtype=float)

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

    # Triangulation -> CW enforcement
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

    output_lines.append("    static constexpr triangle_face_t Triangles[] PROGMEM\n    {")
    processed_triangles: List[Tuple[int, int, int]] = []
    processed_triples_with_materials: List[Tuple[Tuple[IndexTriple, IndexTriple, IndexTriple], Optional[str]]] = []

    # Face normals for emission (may be averaged vertex normals)
    face_norm_vectors: List[np.ndarray] = []

    # For vertex normals accumulation (float)
    vertex_normal_accum = np.zeros((len(processed_vertices), 3), dtype=float)
    vertex_normal_count = np.zeros(len(processed_vertices), dtype=int)

    # Geometric normals stored separately for vertex normal fallback logic
    per_triangle_face_normal: List[np.ndarray] = []

    for (v1i, v2i, v3i), material in triangles_with_materials:
        v1_idx, v2_idx, v3_idx = v1i[0], v2i[0], v3i[0]
        # Enforce CW by swapping second and third
        processed_triangles.append((v1_idx, v3_idx, v2_idx))
        processed_triples_with_materials.append(((v1i, v3i, v2i), material))

        a = processed_vertices[v1_idx]
        b = processed_vertices[v3_idx]
        c = processed_vertices[v2_idx]

        # Geometric (cross product) normal for fallback / vertex normal fallback.
        geo_face_normal = np.cross(b - a, c - a)
        per_triangle_face_normal.append(geo_face_normal)

        # Accumulate vertex normals (if present)
        if emit_vertex_normals and normals:
            for (vidx, _, nidx) in (v1i, v3i, v2i):
                if nidx is not None and 0 <= nidx < len(normals):
                    vn = np.array(normals[nidx], dtype=float)
                    if np.linalg.norm(vn) > 1e-9:
                        vertex_normal_accum[vidx] += vn
                        vertex_normal_count[vidx] += 1

        # Face normal emission:
        # If every corner has an associated vertex normal index, average those vertex normals.
        if emit_face_normals:
            have_all_corner_normals = (
                normals and
                all(trip[2] is not None and 0 <= trip[2] < len(normals) for trip in (v1i, v3i, v2i))
            )
            if have_all_corner_normals:
                n1 = np.array(normals[v1i[2]], dtype=float)
                n2 = np.array(normals[v3i[2]], dtype=float)
                n3 = np.array(normals[v2i[2]], dtype=float)
                averaged = n1 + n2 + n3
                face_norm_vectors.append(averaged)
            else:
                face_norm_vectors.append(geo_face_normal)

    # Emit triangle indices
    for a_idx, b_idx, c_idx in processed_triangles:
        output_lines.append(f"        {{{a_idx}, {b_idx}, {c_idx}}},")
    output_lines.append("    };\n")
    output_lines.append("    constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);\n")

    # Material groups
    output_lines.append("    static constexpr uint8_t Group[TriangleCount] PROGMEM\n    {")
    material_indices = [unique_materials.get(mat, 0) for _, mat in processed_triples_with_materials]
    for i in range(0, len(material_indices), 16):
        output_lines.append("        " + ", ".join(str(x) for x in material_indices[i:i+16]) + ",")
    output_lines.append("    };\n")

    # Vertex normals (quantized magnitude == NORMAL_SCALE)
    if emit_vertex_normals:
        if normals:
            need_fallback = []
            for i in range(len(processed_vertices)):
                if vertex_normal_count[i] == 0:
                    need_fallback.append(i)
                else:
                    n = vertex_normal_accum[i]
                    vertex_normal_accum[i] = normalize_float(n)
        else:
            need_fallback = list(range(len(processed_vertices)))

        if need_fallback:
            for tri_idx, (a_idx, b_idx, c_idx) in enumerate(processed_triangles):
                fn = per_triangle_face_normal[tri_idx]
                fn_unit = normalize_float(fn)
                for vidx in (a_idx, b_idx, c_idx):
                    if vidx in need_fallback:
                        vertex_normal_accum[vidx] += fn_unit
            for vidx in need_fallback:
                n = vertex_normal_accum[vidx]
                vertex_normal_accum[vidx] = normalize_float(n)

        output_lines.append("    static constexpr vertex16_t VertexNormals[] PROGMEM\n    {")
        for i in range(len(processed_vertices)):
            x, y, z = quantize_normal(vertex_normal_accum[i])
            output_lines.append(f"        {{{format_component(x)}, {format_component(y)}, {format_component(z)}}},")
        output_lines.append("    };\n")
        output_lines.append("    constexpr auto VertexNormalCount = sizeof(VertexNormals) / sizeof(vertex16_t);\n")

    # Face normals (quantized magnitude == NORMAL_SCALE)
    if emit_face_normals:
        output_lines.append("    static constexpr vertex16_t FaceNormals[] PROGMEM\n    {")
        for nvec in face_norm_vectors:
            unit = normalize_float(nvec)
            x, y, z = quantize_normal(unit)
            output_lines.append(f"        {{{format_component(x)}, {format_component(y)}, {format_component(z)}}},")
        output_lines.append("    };\n")
        output_lines.append("    constexpr auto FaceNormalCount = sizeof(FaceNormals) / sizeof(vertex16_t);\n")

    # UV emission
    has_texture_dims = texture_width is not None and texture_height is not None
    if emit_uv and has_texture_dims:
        used_uv_indices: List[int] = []
        for (v1i, v2i, v3i), _ in processed_triples_with_materials:
            used_uv_indices.extend([v1i[1], v2i[1], v3i[1]])

        if not texcoords:
            texcoords = [(float(v[0]), float(v[1])) for v in processed_vertices]

        u_values: List[float] = []
        v_values: List[float] = []
        for uv_idx in used_uv_indices:
            if uv_idx is not None and 0 <= uv_idx < len(texcoords):
                uv = texcoords[uv_idx]
                if len(uv) >= 2:
                    u_values.append(uv[0]); v_values.append(uv[1])

        if not u_values or not v_values:
            u_values = [0.0]; v_values = [0.0]

        out_of_range = any(u < 0.0 or u > 1.0 for u in u_values) or any(v < 0.0 or v > 1.0 for v in v_values)
        eps = 1e-6
        all_norm = (not out_of_range and
                    all(-eps <= u <= 1.0 + eps for u in u_values) and
                    all(-eps <= v <= 1.0 + eps for v in v_values))

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

        mode = "wrap" if (uv_wrap_mode == "wrap" or (uv_wrap_mode == "auto" and out_of_range)) else "clamp"

        output_lines.append(f"    static constexpr uint16_t UvMasterWidth = {width};\n")
        output_lines.append(f"    static constexpr uint16_t UvMasterHeight = {height};\n")
        output_lines.append("    static constexpr uint16_t UvCount = TriangleCount * 3;\n")

        u_min, u_max = min(u_values), max(u_values)
        v_min, v_max = min(v_values), max(v_values)
        if abs(u_max - u_min) < 1e-9: u_max = u_min + 1.0
        if abs(v_max - v_min) < 1e-9: v_max = v_min + 1.0

        master_uvs: List[Tuple[int, int]] = []
        for (v1i, v2i, v3i), _ in processed_triples_with_materials:
            for idx_trip in (v1i, v2i, v3i):
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
                        u_norm = wrap01(u_raw); v_norm = wrap01(v_raw)
                    else:
                        u_norm = (u_raw - u_min) / (u_max - u_min)
                        v_norm = (v_raw - v_min) / (v_max - v_min)

                if uv_v_flip:
                    v_norm = 1.0 - v_norm

                u_q = int(round(u_norm * (width - 1)))
                v_q = int(round(v_norm * (height - 1)))
                master_uvs.append((u_q, v_q))

        output_lines.append(f"    static constexpr coordinate_t UVs{width}x{height}[UvCount] PROGMEM\n    {{")
        for u_q, v_q in master_uvs:
            output_lines.append(f"        {{{u_q}, {v_q}}},")
        output_lines.append("    };\n")

        MIN_MIP_DIM = 8

        def level_count(w: int, h: int) -> int:
            cnt = 1
            while w > MIN_MIP_DIM or h > MIN_MIP_DIM:
                mw2 = max(MIN_MIP_DIM, w // 2)
                mh2 = max(MIN_MIP_DIM, h // 2)
                if mw2 == w and mh2 == h:
                    break
                w, h = mw2, mh2
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
                for u_q_master, v_q_master in master_uvs:
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
    print(f"Processed {file_name}: CW triangles={len(processed_triangles)}; "
          f"vertex_normals={'yes' if emit_vertex_normals else 'no'}, face_normals={'yes' if emit_face_normals else 'no'}")
    if emit_face_normals and normals:
        print("  FaceNormals: averaged vertex normals where available.")
    if emit_uv and has_texture_dims:
        print("  UVs: emitted using CW triangle order.")

    return "\n".join(output_lines)
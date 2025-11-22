from typing import List, Tuple, Optional

Vertex = Tuple[float, float, float]
TexCoord = Tuple[float, ...]
Normal = Tuple[float, float, float]
IndexTriple = Tuple[int, Optional[int], Optional[int]]
Face = Tuple[IndexTriple, ...]
FacesWithMaterials = List[Tuple[Face, Optional[str]]]


def read_obj(data: str) -> Tuple[
    List[Vertex],
    List[TexCoord],
    List[Normal],
    FacesWithMaterials,
]:
    """
    Parses OBJ data (supports negative indices) and extracts vertices, texture
    coordinates, normals, and faces with materials.
    """
    vertices: List[Vertex] = []
    texture_coords: List[TexCoord] = []
    normals: List[Normal] = []
    faces_with_materials: FacesWithMaterials = []
    current_material: Optional[str] = None

    lines = data.splitlines()
    for raw in lines:
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        prefix = parts[0]

        if prefix == "v":
            try:
                vertices.append(tuple(map(float, parts[1:4])))  # type: ignore[arg-type]
            except ValueError:
                continue
        elif prefix == "vt":
            try:
                texture_coords.append(tuple(map(float, parts[1:])))
            except ValueError:
                continue
        elif prefix == "vn":
            try:
                normals.append(tuple(map(float, parts[1:4])))  # type: ignore[arg-type]
            except ValueError:
                continue
        elif prefix == "usemtl":
            current_material = parts[1] if len(parts) > 1 else None
        elif prefix == "f":
            face_indices: List[IndexTriple] = []
            try:
                for part in parts[1:]:
                    v_str, vt_str, vn_str = _split_face_token(part)
                    v_idx = _resolve_index(v_str, len(vertices))
                    vt_idx = _resolve_index(vt_str, len(texture_coords)) if vt_str else None
                    vn_idx = _resolve_index(vn_str, len(normals)) if vn_str else None
                    face_indices.append((v_idx, vt_idx, vn_idx))
                faces_with_materials.append((tuple(face_indices), current_material))
            except (ValueError, IndexError):
                continue
        else:
            pass

    return vertices, texture_coords, normals, faces_with_materials


def _split_face_token(token: str) -> Tuple[str, Optional[str], Optional[str]]:
    parts = token.split("/")
    if len(parts) == 1:
        return parts[0], None, None
    if len(parts) == 2:
        return parts[0], parts[1], None
    return parts[0], parts[1], parts[2]


def _resolve_index(raw: str, count: int) -> int:
    """
    OBJ supports negative indices: -1 refers to last defined element.
    """
    idx = int(raw)
    if idx > 0:
        return idx - 1
    # negative
    resolved = count + idx
    if resolved < 0 or resolved >= count:
        raise IndexError("Negative index out of range")
    return resolved
from typing import List, Tuple, Optional

Vertex = Tuple[float, float, float]
TexCoord = Tuple[float, ...]
Normal = Tuple[float, float, float]
IndexTriple = Tuple[int, Optional[int], Optional[int]]
Face = Tuple[IndexTriple, ...]
FacesWithMaterials = List[Tuple[Face, Optional[str]]]


def read_obj(data: str) -> tuple[
    List[Vertex],
    List[TexCoord],
    List[Normal],
    FacesWithMaterials,
]:
    """
    Parses OBJ data and extracts vertices, texture coordinates, normals, and faces,
    associating faces with materials based on 'usemtl' directives.
    """
    vertices: List[Vertex] = []
    texture_coords: List[TexCoord] = []
    normals: List[Normal] = []
    faces_with_materials: FacesWithMaterials = []
    current_material: Optional[str] = None

    for raw in data.splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if not parts:
            continue

        prefix = parts[0]

        if prefix == "v":
            try:
                vertex = tuple(map(float, parts[1:4]))
                vertices.append(vertex)  # type: ignore[arg-type]
            except ValueError:
                continue
        elif prefix == "vt":
            try:
                vt: TexCoord = tuple(map(float, parts[1:]))
                texture_coords.append(vt)
            except ValueError:
                continue
        elif prefix == "vn":
            try:
                normal = tuple(map(float, parts[1:4]))
                normals.append(normal)  # type: ignore[arg-type]
            except ValueError:
                continue
        elif prefix == "usemtl":
            current_material = parts[1] if len(parts) > 1 else None
        elif prefix == "f":
            face_indices: List[IndexTriple] = []
            try:
                for part in parts[1:]:
                    indices = part.split("/")
                    v_idx = int(indices[0]) - 1

                    vt_idx = None
                    if len(indices) > 1 and indices[1]:
                        vt_idx = int(indices[1]) - 1

                    vn_idx = None
                    if len(indices) > 2 and indices[2]:
                        vn_idx = int(indices[2]) - 1

                    face_indices.append((v_idx, vt_idx, vn_idx))
                faces_with_materials.append((tuple(face_indices), current_material))
            except ValueError:
                continue
        else:
            # Ignore other lines
            pass

    return vertices, texture_coords, normals, faces_with_materials
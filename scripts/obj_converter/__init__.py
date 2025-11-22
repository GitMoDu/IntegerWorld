"""
OBJ to custom C++ data generator (Colab-friendly).
"""
# Auto-install numpy if missing (Colab convenience; silent except first install)
try:
    import numpy  # noqa: F401
except Exception:  # broad: handle any import failure
    import sys, subprocess
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "numpy"])
        import numpy  # noqa: F401
    except Exception as exc:  # If install fails, leave it to later imports to raise
        print(f"[obj_converter] Warning: numpy install failed: {exc}")

from .config import OUTPUT_CONFIGURATIONS
from .parser import read_obj
from .converter import convert_to_custom_format
from .fileio import (
    ensure_dir,
    iter_obj_files,
    read_text,
    write_text,
)

def run_all_in_place() -> None:
    """
    Convenience: process all .obj files under ./Input producing ./Output/*.txt
    (All configurations). Intended for Colab notebook use:
        from obj_converter import run_all_in_place
        run_all_in_place()
    """
    import os
    repo_root = os.getcwd()
    input_dir = os.path.join(repo_root, "Input")
    output_dir = os.path.join(repo_root, "Output")

    ensure_dir(input_dir)
    ensure_dir(output_dir)

    pairs = list(iter_obj_files(input_dir))
    if not pairs:
        print("[obj_converter] No .obj files found in Input/. Add files first.")
        return

    for name, path in pairs:
        print(f"[PROCESS] {name}")
        try:
            data = read_text(path)
        except Exception as e:
            print(f"  Read failure: {e}")
            continue

        try:
            vertices, texcoords, normals, faces_with_materials = read_obj(data)
            print(f"  Parsed: V={len(vertices)} VT={len(texcoords)} N={len(normals)} F={len(faces_with_materials)}")
        except Exception as e:
            print(f"  Parse failure: {e}")
            continue

        stem, _ = os.path.splitext(name)
        for cfg in OUTPUT_CONFIGURATIONS:
            ns_name = stem + cfg["name"]
            text = convert_to_custom_format(
                vertices,
                normals,
                faces_with_materials,
                ns_name,
                center_vertices=cfg["center_vertices"],
                apply_winding_normalization=cfg["apply_winding_normalization"],
                invert_winding_logic=cfg["invert_winding_logic"],
                emit_vertex_normals=cfg.get("emit_vertex_normals", False),
                emit_face_normals=cfg.get("emit_face_normals", False),
            )
            out_path = os.path.join(output_dir, f"{stem}{cfg['name']}.txt")
            try:
                write_text(out_path, text)
                print(f"    -> {out_path}")
            except Exception as e:
                print(f"    Write failure: {e}")
    print("[obj_converter] Done.")
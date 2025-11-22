#!/usr/bin/env python3
"""
Colab batch converter.
Processes all .obj files in ./Input and writes results to ./Output using every configuration.
Relies on obj_converter auto-installing numpy if absent.
"""
from __future__ import annotations
import sys
from pathlib import Path
from typing import List, Tuple

from obj_converter import (
    OUTPUT_CONFIGURATIONS,
    read_obj,
    convert_to_custom_format,
    ensure_dir,
    iter_obj_files,
    read_text,
    write_text,
)

INPUT_DIR_NAME = "Input"
OUTPUT_DIR_NAME = "Output"


def process_one(
    name: str,
    path: str,
    output_dir: Path,
) -> Tuple[int, int, int, int]:
    """Parse and convert a single OBJ file across all configurations."""
    try:
        data = read_text(path)
    except Exception as e:
        print(f"  [READ FAIL] {name}: {e}")
        return 0, 0, 0, 0

    try:
        vertices, texcoords, normals, faces_with_materials = read_obj(data)
    except Exception as e:
        print(f"  [PARSE FAIL] {name}: {e}")
        return 0, 0, 0, 0

    print(
        f"  Parsed V={len(vertices)} VT={len(texcoords)} N={len(normals)} F={len(faces_with_materials)}"
    )

    if not faces_with_materials or not vertices:
        print("  [SKIP] Empty geometry or faces.")
        return len(vertices), len(texcoords), len(normals), 0

    stem = Path(name).stem
    triangle_total_for_file = 0

    for cfg in OUTPUT_CONFIGURATIONS:
        ns = stem + cfg["name"]
        text = convert_to_custom_format(
            vertices,
            normals,
            faces_with_materials,
            ns,
            center_vertices=cfg["center_vertices"],
            apply_winding_normalization=cfg["apply_winding_normalization"],
            invert_winding_logic=cfg["invert_winding_logic"],
        )
        if not text.strip():
            print(f"    [WARN] Empty output for config {cfg['name']}")
            continue

        out_path = output_dir / f"{stem}{cfg['name']}.txt"
        try:
            write_text(str(out_path), text)
            print(f"    -> {out_path.name}")
            # Rough triangle count (search line prefix) – avoids re-parsing.
            triangle_count = text.count("},")  # approximate emitted triangles
            triangle_total_for_file += triangle_count
        except Exception as e:
            print(f"    [WRITE FAIL] {out_path.name}: {e}")

    return len(vertices), len(texcoords), len(normals), triangle_total_for_file


def main() -> int:
    print("[IntegerWorld OBJ Batch Converter]")
    cwd = Path(".").resolve()
    input_dir = cwd / INPUT_DIR_NAME
    output_dir = cwd / OUTPUT_DIR_NAME

    ensure_dir(str(input_dir))
    ensure_dir(str(output_dir))

    pairs = list(iter_obj_files(str(input_dir)))
    if not pairs:
        print("No .obj files found in Input/. Add files and re-run.")
        return 10  # distinct code for 'no input'

    files_processed = 0
    total_v = total_vt = total_n = total_tris = 0

    for name, path in pairs:
        print(f"[PROCESS] {name}")
        v, vt, n, tris = process_one(name, path, output_dir)
        if v or vt or n or tris:
            files_processed += 1
        total_v += v
        total_vt += vt
        total_n += n
        total_tris += tris

    print("[SUMMARY]")
    print(f"  Files processed: {files_processed}/{len(pairs)}")
    print(f"  Total vertices:  {total_v}")
    print(f"  Total texcoords: {total_vt}")
    print(f"  Total normals:   {total_n}")
    print(f"  Approx triangles emitted (all configs combined): {total_tris}")
    print("Done.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
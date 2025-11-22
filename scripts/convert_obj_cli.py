#!/usr/bin/env python3
import os
import argparse
from obj_converter import (
    OUTPUT_CONFIGURATIONS,
    read_obj,
    convert_to_custom_format,
    ensure_dir,
    iter_obj_files,
    read_text,
    write_text,
)


def main() -> int:
    parser = argparse.ArgumentParser(description="Convert .obj files to custom C++ arrays.")
    parser.add_argument("--base-dir", default="", help="Base directory containing Input/ and Output/.")
    parser.add_argument("--input-dir", default="", help="Override input directory.")
    parser.add_argument("--output-dir", default="", help="Override output directory.")
    parser.add_argument("--file", default="", help="Process only this .obj file name.")
    parser.add_argument("--config-filter", default="", help="Substring filter on configuration name (e.g. _raw_).")
    args = parser.parse_args()

    input_dir = args.input_dir or (os.path.join(args.base_dir, "Input") if args.base_dir else os.path.join(os.getcwd(), "Input"))
    output_dir = args.output_dir or (os.path.join(args.base_dir, "Output") if args.base_dir else os.path.join(os.getcwd(), "Output"))

    if not os.path.isdir(input_dir):
        print(f"Error: Input directory not found: {input_dir}")
        return 2

    ensure_dir(output_dir)

    pairs = list(iter_obj_files(input_dir))
    if args.file:
        pairs = [(n, p) for (n, p) in pairs if n == args.file]
        if not pairs:
            print(f"Error: File {args.file} not found in {input_dir}")
            return 3

    active_configs = OUTPUT_CONFIGURATIONS
    if args.config_filter:
        active_configs = [c for c in OUTPUT_CONFIGURATIONS if args.config_filter in c["name"]]
        if not active_configs:
            print(f"No configurations match filter: {args.config_filter}")
            return 4

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

        if not faces_with_materials or not vertices:
            print("  [SKIP] Empty geometry or faces.")
            continue

        stem, _ = os.path.splitext(name)
        for cfg in active_configs:
            out_ns = stem + cfg["name"]
            text = convert_to_custom_format(
                vertices,
                normals,
                faces_with_materials,
                out_ns,
                center_vertices=cfg["center_vertices"],
                apply_winding_normalization=cfg["apply_winding_normalization"],
                invert_winding_logic=cfg["invert_winding_logic"],
                emit_vertex_normals=cfg.get("emit_vertex_normals", False),
                emit_face_normals=cfg.get("emit_face_normals", False),
            )
            out_file = os.path.join(output_dir, f"{stem}{cfg['name']}.txt")
            try:
                write_text(out_file, text)
                print(f"    -> {out_file}")
            except Exception as e:
                print(f"    Write failure: {e}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
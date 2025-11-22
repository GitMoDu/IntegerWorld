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
    mount_colab_if_requested,
)


def main() -> int:
    parser = argparse.ArgumentParser(description="Convert .obj files to custom C++ arrays.")
    parser.add_argument("--base-dir", default="", help="Base directory. If set, uses <base>/Input and <base>/Output.")
    parser.add_argument("--input-dir", default="", help="Input directory containing .obj files. Overrides --base-dir if set.")
    parser.add_argument("--output-dir", default="", help="Output directory for generated .txt files. Overrides --base-dir if set.")
    parser.add_argument("--colab", action="store_true", help="Mount Google Drive in Colab at /content/drive.")
    parser.add_argument("--file", default="", help="Optional specific .obj file name to process from the input directory.")
    args = parser.parse_args()

    mount_colab_if_requested(args.colab)

    # Resolve directories
    if args.input_dir:
        input_dir = args.input_dir
    elif args.base_dir:
        input_dir = os.path.join(args.base_dir, "Input")
    else:
        input_dir = os.path.join(os.getcwd(), "Input")

    if args.output_dir:
        output_dir = args.output_dir
    elif args.base_dir:
        output_dir = os.path.join(args.base_dir, "Output")
    else:
        output_dir = os.path.join(os.getcwd(), "Output")

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

    for name, path in pairs:
        print(f"Processing file: {name}")
        try:
            data = read_text(path)
        except Exception as e:
            print(f"Failed to read {path}: {e}")
            continue

        try:
            vertices, texture_coords, normals, faces_with_materials = read_obj(data)
            print(
                f"Parsed {len(vertices)} vertices, {len(texture_coords)} texture coordinates, "
                f"{len(normals)} normals, and {len(faces_with_materials)} faces."
            )
        except Exception as e:
            print(f"Failed to parse {name}: {e}")
            continue

        stem, _ = os.path.splitext(name)

        for cfg in OUTPUT_CONFIGURATIONS:
            suffix = cfg["name"]
            out_ns = stem + suffix
            out_text = convert_to_custom_format(
                vertices,
                normals,
                faces_with_materials,
                out_ns,
                center_vertices=cfg["center_vertices"],
                apply_winding_normalization=cfg["apply_winding_normalization"],
                invert_winding_logic=cfg["invert_winding_logic"],
            )

            out_file = os.path.join(output_dir, f"{stem}{suffix}.txt")
            try:
                write_text(out_file, out_text)
                print(f"Wrote: {out_file}")
            except Exception as e:
                print(f"Failed to write {out_file}: {e}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
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

try:
    from obj_converter.imageinfo import get_image_size
except Exception:
    get_image_size = None  # type: ignore[assignment]


def _find_texture_for_obj(obj_path: str):
    directory, filename = os.path.split(obj_path)
    stem, _ = os.path.splitext(filename)
    candidates = [
        os.path.join(directory, stem + ".png"),
        os.path.join(directory, stem + ".jpg"),
        os.path.join(directory, stem + ".jpeg"),
    ]
    if get_image_size:
        for c in candidates:
            dims = get_image_size(c)
            if dims:
                return dims
    return None, None


def main() -> int:
    parser = argparse.ArgumentParser(description="Convert .obj files to custom C++ arrays.")
    parser.add_argument("--base-dir", default="", help="Base directory containing Input/ and Output/.")
    parser.add_argument("--input-dir", default="", help="Override input directory.")
    parser.add_argument("--output-dir", default="", help="Override output directory.")
    parser.add_argument("--file", default="", help="Process only this .obj file name.")
    parser.add_argument("--config-filter", default="", help="Substring filter on configuration name (e.g. _raw).")
    parser.add_argument("--no-uv-mips", action="store_true", help="Do not emit UV mip levels.")
    parser.add_argument("--no-force-pow2", action="store_true", help="Use actual texture size (no power-of-two upscaling).")
    parser.add_argument("--no-uv", action="store_true", help="Disable UV emission regardless of texture presence.")
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

        tex_w, tex_h = _find_texture_for_obj(path)
        if tex_w and tex_h and not args.no_uv:
            print(f"  Texture: {tex_w}x{tex_h} (source image)")
            emit_uv = True
        else:
            if args.no_uv:
                print("  Texture: UV emission disabled by flag")
            else:
                print("  Texture: not found -> UVs will be skipped")
            emit_uv = False

        stem, _ = os.path.splitext(name)
        object_out_dir = os.path.join(output_dir, stem)
        ensure_dir(object_out_dir)
        print(f"  Output folder: {object_out_dir}")

        for cfg in active_configs:
            out_ns = stem + cfg["name"]
            text = convert_to_custom_format(
                vertices=vertices,
                texcoords=texcoords,
                normals=normals,
                faces_with_materials=faces_with_materials,
                file_name=out_ns,
                center_vertices=cfg["center_vertices"],
                emit_vertex_normals=cfg.get("emit_vertex_normals", False),
                emit_face_normals=cfg.get("emit_face_normals", False),
                emit_uv=(emit_uv and cfg.get("emit_uv", False)),
                emit_uv_mips=(not args.no_uv_mips) and cfg.get("emit_uv_mips", False),
                texture_width=tex_w,
                texture_height=tex_h,
                uv_force_pow2=not args.no_force_pow2,
                # Using converter defaults for: uv_v_flip=True, uv_wrap_mode="auto"
            )
            out_file = os.path.join(object_out_dir, f"{stem}{cfg['name']}.txt")
            try:
                write_text(out_file, text)
                print(f"    -> {out_file}")
            except Exception as e:
                print(f"    Write failure: {e}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
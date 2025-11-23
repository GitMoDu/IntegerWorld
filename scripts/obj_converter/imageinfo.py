import os
from typing import Optional, Tuple

def _read_uint32_be(b: bytes) -> int:
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]

def get_image_size(path: str) -> Optional[Tuple[int, int]]:
    """
    Read JPEG or PNG dimensions. Returns (width, height) or None if unsupported/failed.
    """
    if not os.path.isfile(path):
        return None
    try:
        with open(path, "rb") as f:
            sig = f.read(16)

            # PNG
            if sig.startswith(b"\x89PNG\r\n\x1a\n"):
                # IHDR chunk starts at byte 8; structure: length(4) 'IHDR'(4) width(4) height(4)
                # We already read first 16 bytes; read width/height at positions 16..24
                f.seek(16)
                ihdr = f.read(13)  # IHDR chunk data
                if len(ihdr) >= 8:
                    width = _read_uint32_be(ihdr[0:4])
                    height = _read_uint32_be(ihdr[4:8])
                    return width, height
                return None

            # JPEG
            if sig[0:2] == b"\xFF\xD8":
                f.seek(2)
                while True:
                    marker_prefix = f.read(1)
                    if not marker_prefix:
                        break
                    if marker_prefix != b"\xFF":
                        # Skip padding bytes
                        continue
                    marker_type = f.read(1)
                    if not marker_type or marker_type == b"\xD9":  # EOI
                        break
                    if marker_type in (b"\xC0", b"\xC1", b"\xC2", b"\xC3", b"\xC5", b"\xC6", b"\xC7", b"\xC9", b"\xCA", b"\xCB", b"\xCD", b"\xCE", b"\xCF"):
                        # SOF: next two bytes length, then precision(1), height(2), width(2)
                        length_bytes = f.read(2)
                        if len(length_bytes) != 2:
                            break
                        seg_len = (length_bytes[0] << 8) | length_bytes[1]
                        data = f.read(seg_len - 2)
                        if len(data) >= 7:
                            height = (data[1] << 8) | data[2]
                            width = (data[3] << 8) | data[4]
                            return width, height
                        break
                    else:
                        length_bytes = f.read(2)
                        if len(length_bytes) != 2:
                            break
                        seg_len = (length_bytes[0] << 8) | length_bytes[1]
                        f.seek(seg_len - 2, os.SEEK_CUR)
            return None
    except Exception:
        return None
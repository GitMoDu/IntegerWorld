import os
from typing import Iterable, Tuple, Optional


def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def iter_obj_files(input_dir: str) -> Iterable[Tuple[str, str]]:
    for name in sorted(os.listdir(input_dir)):
        if name.lower().endswith(".obj"):
            yield name, os.path.join(input_dir, name)


def read_text(path: str) -> str:
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def write_text(path: str, data: str) -> None:
    with open(path, "w", encoding="utf-8") as f:
        f.write(data)


def mount_colab_if_requested(enable: bool) -> bool:
    """
    Mount Google Drive in Colab when requested.
    Returns True if mounted, False otherwise.
    """
    if not enable:
        return False
    try:
        from google.colab import drive  # type: ignore
        if not os.path.isdir("/content/drive"):
            print("Mounting Google Drive...")
            drive.mount("/content/drive")
        else:
            print("Google Drive is already mounted.")
        return True
    except Exception as exc:
        print(f"Colab mount not available: {exc}")
        return False
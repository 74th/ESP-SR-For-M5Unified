#!/usr/bin/env python3
"""Create an Arduino IDE import ZIP for this library.

Usage:
  python scripts/make_arduino_zip.py
  python scripts/make_arduino_zip.py --output-dir ./dist
  python scripts/make_arduino_zip.py --include-platformio-examples
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


HEADER_EXTENSIONS = {".h", ".hpp", ".hh", ".hxx"}


def parse_library_properties(path: Path) -> dict[str, str]:
    props: dict[str, str] = {}
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        props[key.strip()] = value.strip()
    return props


def sanitize_for_filename(value: str) -> str:
    safe = re.sub(r"[^A-Za-z0-9._-]+", "-", value.strip())
    return safe.strip("-._") or "library"


def should_include_example_dir(path: Path, include_platformio_examples: bool) -> bool:
    if include_platformio_examples:
        return True
    return not path.name.endswith("_platformio")


def collect_files(repo_root: Path, include_platformio_examples: bool) -> list[Path]:
    files: list[Path] = []

    top_level_files = [
        "library.properties",
        "library.json",
        "LICENSE.md",
        "README.md",
        "README_ja.md",
    ]

    for name in top_level_files:
        p = repo_root / name
        if p.exists() and p.is_file():
            files.append(p)

    for dirname in ("src", "include"):
        base = repo_root / dirname
        if base.exists() and base.is_dir():
            files.extend([p for p in base.rglob("*") if p.is_file()])

    examples_dir = repo_root / "examples"
    if examples_dir.exists() and examples_dir.is_dir():
        for child in sorted(examples_dir.iterdir()):
            if not child.is_dir():
                continue
            if not should_include_example_dir(child, include_platformio_examples):
                continue
            files.extend([p for p in child.rglob("*") if p.is_file()])

    return sorted(set(files))


def build_zip(
    repo_root: Path,
    output_dir: Path,
    archive_name: str | None,
    include_platformio_examples: bool,
) -> Path:
    props_path = repo_root / "library.properties"
    if not props_path.exists():
        raise FileNotFoundError(f"library.properties not found: {props_path}")

    props = parse_library_properties(props_path)
    library_name = sanitize_for_filename(props.get("name", repo_root.name))
    version = sanitize_for_filename(props.get("version", "unknown"))

    if archive_name:
        zip_filename = archive_name if archive_name.endswith(".zip") else f"{archive_name}.zip"
    else:
        zip_filename = f"{library_name}-{version}-arduino.zip"

    output_dir.mkdir(parents=True, exist_ok=True)
    zip_path = output_dir / zip_filename

    files = collect_files(repo_root, include_platformio_examples)
    if not files:
        raise RuntimeError("No files matched for packaging.")

    with ZipFile(zip_path, "w", compression=ZIP_DEFLATED) as zf:
        written_entries: set[str] = set()

        for file_path in files:
            rel = file_path.relative_to(repo_root)
            arcname = Path(library_name) / rel
            arcname_str = arcname.as_posix()
            zf.write(file_path, arcname_str)
            written_entries.add(arcname_str)

        # Arduino IDE's library validation expects header files in src/ (or root).
        # This repository keeps headers in include/, so mirror those headers to src/
        # inside the ZIP for import compatibility.
        include_dir = repo_root / "include"
        if include_dir.exists() and include_dir.is_dir():
            for header in sorted(include_dir.rglob("*")):
                if not header.is_file() or header.suffix.lower() not in HEADER_EXTENSIONS:
                    continue
                include_rel = header.relative_to(include_dir)
                src_arcname = (Path(library_name) / "src" / include_rel).as_posix()
                if src_arcname in written_entries:
                    continue
                zf.write(header, src_arcname)
                written_entries.add(src_arcname)

    return zip_path


def main() -> int:
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent

    parser = argparse.ArgumentParser(
        description="Create an Arduino IDE import ZIP for this repository."
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=repo_root / "dist",
        help="Directory to place the ZIP file (default: ./dist)",
    )
    parser.add_argument(
        "--name",
        type=str,
        default=None,
        help="ZIP file name (without or with .zip).",
    )
    parser.add_argument(
        "--include-platformio-examples",
        action="store_true",
        help="Include examples/*_platformio in the archive.",
    )

    args = parser.parse_args()

    try:
        zip_path = build_zip(
            repo_root=repo_root,
            output_dir=args.output_dir,
            archive_name=args.name,
            include_platformio_examples=args.include_platformio_examples,
        )
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    print(f"Created: {zip_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

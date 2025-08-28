#!/usr/bin/env python3
"""
Orchestrator wrapper for the smaller converter modules in `tools/`.
This script keeps the same CLI as before but delegates actual work to
`pc_convert`, `pc_io`, `pc_imaging`, etc.

Usage:
  python3 pokemon_converter.py <input_file> [output_png] [output_raw] [metadata_json]

The script returns exit code 0 on success, non-zero on failure.
"""
import sys
import json
import os

from pc_convert import convert_single
from pc_utils import ensure_dir


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 pokemon_converter.py <input_file> [output_png] [output_raw] [metadata_json]")
        return 1

    input_file = sys.argv[1]
    output_png = None
    output_raw = None
    metadata = None

    if len(sys.argv) >= 3 and sys.argv[2] and sys.argv[2] != 'None':
        output_png = sys.argv[2]
    if len(sys.argv) >= 4 and sys.argv[3] and sys.argv[3] != 'None':
        output_raw = sys.argv[3]
    if len(sys.argv) >= 5:
        try:
            metadata = json.loads(sys.argv[4])
            print(f"   📋 Using metadata: {metadata.get('name', 'Unknown')}")
        except Exception as e:
            print(f"   ⚠️  Invalid metadata JSON: {e}")

    # Ensure output dirs
    if output_png:
        ensure_dir(os.path.dirname(output_png))
    if output_raw:
        ensure_dir(os.path.dirname(output_raw))

    try:
        final_img, meta = convert_single(input_file, output_png, output_raw)
        print(f"🔄 Converted: {input_file}")
        if output_png:
            print(f"   ✅ Saved PNG: {output_png}")
        if output_raw:
            print(f"   ✅ Saved RAW: {output_raw}")
        return 0
    except Exception as e:
        print(f"   ❌ Conversion failed: {e}")
        return 2


if __name__ == '__main__':
    sys.exit(main())

#!/usr/bin/env python3
"""Orchestrator for the Pokemon Expositor image conversion pipeline.

This script is intentionally small: it parses CLI arguments and delegates
work to the modular implementation in the same folder (pc_convert, pc_io, etc.).
"""
import sys
import json
from pc_convert import convert_single


def usage():
    print("Usage:")
    print("  python3 pokemon_converter.py <input_file> [output_png] [output_raw]")
    print("  python3 pokemon_converter.py <input_file> [output_png] [output_raw] [metadata_json]")


def main():
    if len(sys.argv) < 2:
        usage()
        return

    input_file = sys.argv[1]
    output_png = None
    output_raw = None
    metadata = None

    if len(sys.argv) >= 3:
        output_png = sys.argv[2]
    if len(sys.argv) >= 4:
        output_raw = sys.argv[3]
    if len(sys.argv) >= 5:
        try:
            metadata = json.loads(sys.argv[4])
        except Exception:
            print("Warning: invalid metadata JSON, ignoring")

    print(f"Processing: {input_file}")
    try:
        final_img, meta = convert_single(input_file, output_png=output_png, output_raw=output_raw)
        print("Conversion OK")
        print("Meta:", meta)
    except Exception as e:
        print("Conversion failed:", e)


if __name__ == '__main__':
    main()

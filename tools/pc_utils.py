"""Utility helpers for pokemon_converter modules."""
import struct
import os
from pc_constants import TARGET_WIDTH, TARGET_HEIGHT, BYTES_PER_PIXEL

def rgb888_to_rgb565_bytes(r, g, b):
    """Convert 8-bit RGB to little-endian RGB565 bytes."""
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    rgb565 = (r5 << 11) | (g6 << 5) | b5
    return struct.pack('<H', rgb565)

def ensure_dir(path):
    if not path:
        return
    os.makedirs(path, exist_ok=True)

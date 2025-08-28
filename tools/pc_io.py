"""IO helpers: save PNG and RAW RGB565 files."""
from pc_constants import TARGET_WIDTH, TARGET_HEIGHT, BYTES_PER_PIXEL
from pc_utils import rgb888_to_rgb565_bytes

def save_png(image, path):
    image.save(path, 'PNG')

def save_raw_rgb565(image, path):
    # image is PIL RGB with TARGET_WIDTH x TARGET_HEIGHT
    pixels = image.load()
    with open(path, 'wb') as f:
        for y in range(TARGET_HEIGHT):
            for x in range(TARGET_WIDTH):
                r, g, b = pixels[x, y]
                f.write(rgb888_to_rgb565_bytes(r, g, b))

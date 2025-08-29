"""High-level conversion orchestrator for a single image.
Uses pc_imaging and pc_io modules.
"""
from PIL import Image
from pc_imaging import compose_final_image
from pc_io import save_png, save_raw_rgb565

def convert_single(input_path, output_png=None, output_raw=None, metadata=None):
    img = Image.open(input_path)
    if img.mode != 'RGB':
        img = img.convert('RGB')
    img_rotated = img.rotate(90, expand=True)
    final_img, meta = compose_final_image(img_rotated, metadata)
    if output_png:
        save_png(final_img, output_png)
    if output_raw:
        save_raw_rgb565(final_img, output_raw)
    return final_img, meta

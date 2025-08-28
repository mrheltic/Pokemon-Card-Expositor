"""Image processing helpers for the converter.
Contains composition and resizing utilities.
"""
from PIL import Image, ImageFilter
from pc_constants import TARGET_WIDTH, TARGET_HEIGHT

def create_blurred_background(img_rotated):
    # Create blurred background from rotated image
    blur_scale = 0.3
    small_size = (max(1, int(img_rotated.width * blur_scale)), max(1, int(img_rotated.height * blur_scale)))
    img_small = img_rotated.resize(small_size, Image.Resampling.LANCZOS)
    img_blurred = img_small.filter(ImageFilter.GaussianBlur(radius=8))
    img_blurred_large = img_blurred.resize((TARGET_WIDTH, TARGET_HEIGHT), Image.Resampling.LANCZOS)
    overlay = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), (0, 0, 0))
    return Image.blend(img_blurred_large, overlay, 0.7)

def compose_final_image(img_rotated):
    """Resize rotated image to fit into target and compose final framed image.
    Returns the final PIL.Image and metadata (new_width, new_height, x_offset, y_offset, scale_factor).
    """
    rot_width, rot_height = img_rotated.size
    scale_x = TARGET_WIDTH / rot_width
    scale_y = TARGET_HEIGHT / rot_height
    scale_factor = min(scale_x, scale_y)
    new_width = max(1, int(rot_width * scale_factor))
    new_height = max(1, int(rot_height * scale_factor))
    img_resized = img_rotated.resize((new_width, new_height), Image.Resampling.LANCZOS)

    # Create background from blurred and compose
    final_img = create_blurred_background(img_rotated)

    x_offset = TARGET_WIDTH - new_width
    y_offset = 0
    final_img.paste(img_resized, (x_offset, y_offset))

    return final_img, (new_width, new_height, x_offset, y_offset, scale_factor)

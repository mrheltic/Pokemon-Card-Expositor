"""Image processing helpers for the converter.
Contains composition and resizing utilities.
"""
from PIL import Image, ImageFilter, ImageDraw, ImageFont
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

def add_text_to_background(background_img, metadata, x_offset, side='left'):
    """Add card information text to the background image.
    Text is anchored exactly at the left border, sized to fit the left margin and rotated to match display orientation.
    """
    if not metadata:
        return background_img
    
    img_with_text = background_img.copy()

    # Base font size reduced by 30% as requested
    base_font_size = max(10, int(48 * 0.7))  # 48 -> ~33
    font = None
    try:
        font = ImageFont.truetype("DejaVuSans-Bold.ttf", base_font_size)
    except Exception:
        try:
            font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", base_font_size)
        except Exception:
            font = ImageFont.load_default()

    text_color = (255, 255, 255)

    # Collect lines
    lines = []
    if 'name' in metadata:
        lines.append(metadata['name'])
    if 'set' in metadata:
        lines.append(metadata['set'])
    if 'year' in metadata:
        lines.append(metadata['year'])
    if 'rarity' in metadata:
        lines.append(metadata['rarity'])
    # artist intentionally removed per request

    if not lines:
        return background_img

    # Build a base canvas sized so that after rotation the short side equals left_margin
    left_margin = max(10, x_offset)
    base_h = max(40, min(left_margin, TARGET_HEIGHT - 10))
    base_w = max(200, min(TARGET_HEIGHT - 10, x_offset * 2))  # wide enough to hold long lines before rotation

    text_img = Image.new('RGBA', (base_w, base_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(text_img)

    # Vertical padding and usable height (reduced to make lines closer)
    pad_v = 2
    usable_h = max(10, base_h - 2 * pad_v)

    # Measure approx_line_h now to set tight interline spacing
    try:
        bbox0 = draw.textbbox((0, 0), lines[0], font=font)
        approx_line_h = bbox0[3] - bbox0[1]
    except Exception:
        approx_line_h = int(getattr(font, 'size', base_font_size) * 1.0)

    # Very tight vertical spacing: approx line height plus a small gap
    gap = max(0, int(base_font_size * 0.02))
    vertical_spacing = max(1, approx_line_h + gap)

    # Measure and adjust font if line height is too big
    try:
        bbox = draw.textbbox((0, 0), lines[0], font=font)
        approx_line_h = bbox[3] - bbox[1]
    except Exception:
        approx_line_h = int(getattr(font, 'size', base_font_size) * 1.2)

    if approx_line_h + 4 > vertical_spacing:
        scale = (vertical_spacing - 4) / approx_line_h
        scale = max(0.5, min(scale, 1.0))
        try:
            new_size = max(10, int(base_font_size * scale))
            font = ImageFont.truetype("DejaVuSans-Bold.ttf", new_size)
        except Exception:
            try:
                font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", new_size)
            except Exception:
                font = ImageFont.load_default()
        try:
            bbox = draw.textbbox((0, 0), lines[0], font=font)
            approx_line_h = bbox[3] - bbox[1]
        except Exception:
            pass

    # Draw starting at x=0 so the text block is as far left as possible.
    # Cropping later keeps a small safety margin so glyphs aren't clipped.
    left_draw_x = 0

    # Draw lines with negative x so glyphs can extend to the very left after crop
    y = pad_v
    for line in lines:
        draw.text((left_draw_x, y), line, fill=text_color, font=font)
        y += vertical_spacing

    # Rotate the text canvas 90 degrees clockwise
    rotated_text = text_img.rotate(90, expand=True)

    # Crop transparent borders but keep a small padding so glyphs aren't cut
    bbox = rotated_text.getbbox()
    if bbox:
        # Use asymmetric crop padding: keep minimal/zero left padding so text can sit at the extreme left,
        # while keeping a small pad on right/bottom to avoid tight cropping.
        pad_px = max(3, int(base_font_size * 0.12))
        left_crop_pad = 0
        top_crop_pad = max(2, int(base_font_size * 0.08))
        right_crop_pad = pad_px
        bottom_crop_pad = pad_px
        x0 = max(0, bbox[0] - left_crop_pad)
        y0 = max(0, bbox[1] - top_crop_pad)
        x1 = min(rotated_text.width, bbox[2] + right_crop_pad)
        y1 = min(rotated_text.height, bbox[3] + bottom_crop_pad)
        rotated_text = rotated_text.crop((x0, y0, x1, y1))

    # Ensure final rotated width does not exceed left_margin and short side <= 660
    max_short_side = 660
    desired_width = max(1, left_margin - 2)

    # If rotated_text is too wide, scale down; if it's too narrow, scale up to fill margin
    scale_w = desired_width / rotated_text.width if rotated_text.width > 0 else 1.0
    scale_h = max_short_side / rotated_text.height if rotated_text.height > 0 else 1.0

    # We allow scaling up but cap to not exceed TARGET_HEIGHT or max_short_side
    final_scale = min(scale_w if scale_w > 1.0 else 1.0, scale_h, TARGET_HEIGHT / rotated_text.height, 2.0)
    # If it's wider than desired, scale down instead
    if rotated_text.width > desired_width:
        down_scale = desired_width / rotated_text.width
        final_scale = min(final_scale, down_scale)

    if final_scale != 1.0:
        new_w = max(1, int(rotated_text.width * final_scale))
        new_h = max(1, int(rotated_text.height * final_scale))
        rotated_text = rotated_text.resize((new_w, new_h), Image.Resampling.LANCZOS)

    # After scaling, if still slightly off, force width exact to desired_width by stretching horizontally
    if rotated_text.width < desired_width:
        # Stretch horizontally to exactly desired_width while keeping height
        stretched = rotated_text.resize((desired_width, rotated_text.height), Image.Resampling.LANCZOS)
        rotated_text = stretched

    # Paste at the chosen side using x_offset as the reference margin so text aligns to the card edge:
    # - for 'left': align rotated_text right edge to x_offset (left margin)
    # - for 'right': align rotated_text left edge to x_offset (right margin)
    if side == 'right':
        # x_offset here is the right margin (space to the right of the card)
        x_start = min(max(0, int(x_offset)), TARGET_WIDTH - rotated_text.width)
    else:
        # x_offset here is the left margin (space to the left of the card)
        x_start = max(0, int(x_offset) - rotated_text.width)
    y_start = 0
    img_with_text.paste(rotated_text, (x_start, y_start), rotated_text)

    return img_with_text

def compose_final_image(img_rotated, metadata=None):
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

    # Create background from blurred. We paste the card on the right and place text on the left.
    final_img = create_blurred_background(img_rotated)
    x_offset = TARGET_WIDTH - new_width
    # Paste card at right
    y_offset = 0
    final_img.paste(img_resized, (x_offset, y_offset))
    # Compute left margin for text: space to the left of the pasted card
    left_margin = x_offset
    final_img = add_text_to_background(final_img, metadata, left_margin, side='left')

    return final_img, (new_width, new_height, x_offset, y_offset, scale_factor)

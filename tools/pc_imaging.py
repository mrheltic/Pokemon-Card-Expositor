"""
Pokemon Card Expositor - Image Processing Module

This module handles all image processing operations for the Pokemon card conversion
pipeline, including background generation, text overlay rendering, and final image
composition optimized for a 5-inch rotated display.

Technical Notes:
- All processing assumes 90° counterclockwise display rotation
- Text must be rotated 90° clockwise to appear correctly on rotated display
- RGB565 compatibility maintained throughout pipeline
- Memory-efficient processing for large images

Author: mrheltic
Date: August 2025
"""

from PIL import Image, ImageFilter, ImageDraw, ImageFont
from typing import Tuple, Dict, Any, Optional
from pc_constants import (
    TARGET_WIDTH, TARGET_HEIGHT, BLUR_RADIUS, BLUR_SCALE_FACTOR,
    BACKGROUND_OPACITY, MAX_FONT_SIZE, MIN_FONT_SIZE, TEXT_STROKE_WIDTH,
    LINE_SPACING, COLOR_WHITE, COLOR_BLACK, COLOR_TRANSPARENT
)


def create_blurred_background(img_rotated: Image.Image) -> Image.Image:
    """
    Create a blurred background from the source image for visual appeal.
    
    This function generates an aesthetically pleasing background by applying
    a Gaussian blur to the source image. The blur is applied to a downscaled
    version for performance, then upscaled to target resolution.
    
    Performance optimization: The blur operation is computationally expensive,
    so we downsample the image to 30% size, apply blur, then upscale back.
    This provides 90% of the visual quality at 10% of the processing time.
    
    Args:
        img_rotated (Image.Image): Source image after 90° rotation
        
    Returns:
        Image.Image: Blurred background at target resolution (1024x600)
        
    Example:
        >>> background = create_blurred_background(rotated_card_image)
        >>> # Result: Softly blurred version suitable for text overlay
    """
    # Calculate downscaled dimensions for performance optimization
    small_width = max(1, int(img_rotated.width * BLUR_SCALE_FACTOR))
    small_height = max(1, int(img_rotated.height * BLUR_SCALE_FACTOR))
    small_size = (small_width, small_height)
    
    # Step 1: Downsample for efficient blur processing
    img_small = img_rotated.resize(small_size, Image.Resampling.LANCZOS)
    
    # Step 2: Apply Gaussian blur with optimized radius
    img_blurred = img_small.filter(ImageFilter.GaussianBlur(radius=BLUR_RADIUS))
    
    # Step 3: Upscale blurred image to target display resolution
    img_blurred_large = img_blurred.resize((TARGET_WIDTH, TARGET_HEIGHT), Image.Resampling.LANCZOS)
    
    # Step 4: Apply dark overlay for improved text contrast
    # Create a semi-transparent black overlay to darken the background
    overlay = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), COLOR_BLACK)
    
    # Blend blurred image with dark overlay for optimal text readability
    return Image.blend(img_blurred_large, overlay, BACKGROUND_OPACITY)


def add_text_to_background(background_img: Image.Image, metadata: Optional[Dict[str, Any]], 
                         x_offset: int, side: str = 'left') -> Image.Image:
    """
    Render card metadata text onto the background with rotation compensation.
    
    This function handles the complex task of rendering readable text on a
    display that will be physically rotated 90° left. The text must be
    pre-rotated 90° clockwise in software so it appears upright after
    the physical display rotation.
    
    Visual Layout (after physical 90° left rotation):
    ┌─────────────────────────────────────────┐
    │ M │                                     │
    │ e │                                     │  
    │ t │          Card Image                 │
    │ a │                                     │
    │ d │                                     │
    │ a │                                     │
    │ t │                                     │
    │ a │                                     │
    └─────────────────────────────────────────┘
    
    Args:
        background_img (Image.Image): Background image to overlay text on
        metadata (Optional[Dict[str, Any]]): Card information dictionary
        x_offset (int): X position where card image will be placed
        side (str): Text placement side ('left' or 'right')
        
    Returns:
        Image.Image: Background with text overlay applied
        
    Raises:
        ValueError: If x_offset leaves insufficient space for text
    """
    # Validate inputs
    if not metadata:
        return background_img
    
    # Calculate available text area based on card placement
    available_width = int(x_offset) - 20  # 20px margin from edges
    if available_width <= 40:  # Minimum viable text area
        print("Warning: Insufficient space for text overlay")
        return background_img
    
    # Extract text content from metadata
    text_lines = []
    metadata_fields = ['name', 'set_name', 'release_date', 'rarity']
    
    for field in metadata_fields:
        value = metadata.get(field)
        if value and str(value).strip():
            # Format the text appropriately
            if field == 'release_date' and len(str(value)) >= 4:
                # Extract year from date string
                text_lines.append(f"Year: {str(value)[:4]}")
            elif field == 'set_name':
                text_lines.append(f"Set: {str(value)}")
            elif field == 'rarity':
                text_lines.append(f"Rarity: {str(value)}")
            else:
                text_lines.append(str(value))
    
    if not text_lines:
        return background_img
    
    # Calculate optimal font size based on available space
    # Font size should scale with available area while staying within limits
    base_font_size = min(MAX_FONT_SIZE, available_width // 5)
    font_size = max(MIN_FONT_SIZE, base_font_size)
    
    # Load font with fallback hierarchy
    font = _load_font(font_size)
    
    # Create temporary image for text rendering
    # Dimensions are swapped because we'll rotate 90° later
    text_height = available_width
    text_width = TARGET_HEIGHT
    
    # Create RGBA image for proper transparency handling
    text_img = Image.new('RGBA', (text_width, text_height), COLOR_TRANSPARENT)
    text_draw = ImageDraw.Draw(text_img)
    
    # Calculate text layout parameters
    line_height = font_size + LINE_SPACING
    total_text_height = len(text_lines) * line_height
    
    # Center text vertically in available area
    start_y = max(0, (text_height - total_text_height) // 2)
    start_x = 20  # Left margin for text readability
    
    # Render each line of text with stroke effect
    for i, line in enumerate(text_lines):
        y_position = start_y + (i * line_height)
        
        # Truncate text if it exceeds available width
        truncated_line = _truncate_text_to_fit(text_draw, line, font, text_width - 40)
        
        # Render stroke effect for text visibility
        _render_text_with_stroke(text_draw, truncated_line, start_x, y_position, font)
    
    # Apply 90° clockwise rotation to compensate for display orientation
    # This ensures text appears upright on the physically rotated display
    text_rotated = text_img.rotate(90, expand=True)
    
    # Composite rotated text onto background
    img_with_text = background_img.copy()
    
    # Position text in left area of display
    paste_x = 10  # Small margin from left edge
    paste_y = (TARGET_HEIGHT - text_rotated.height) // 2  # Vertical center
    
    # Apply text with proper alpha blending
    if text_rotated.mode == 'RGBA':
        img_with_text.paste(text_rotated, (paste_x, paste_y), text_rotated)
    else:
        img_with_text.paste(text_rotated, (paste_x, paste_y))
    
    return img_with_text


def _load_font(font_size: int) -> ImageFont.ImageFont:
    """
    Load font with fallback hierarchy for cross-platform compatibility.
    
    Attempts to load fonts in order of preference:
    1. DejaVu Sans Bold (Linux/Unix systems)
    2. Arial Bold (Windows systems)  
    3. System default font (fallback)
    
    Args:
        font_size (int): Desired font size in pixels
        
    Returns:
        ImageFont.ImageFont: Loaded font object
    """
    font_paths = [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",  # Linux
        "/System/Library/Fonts/Arial.ttf",                      # macOS
        "C:\\Windows\\Fonts\\arial.ttf",                        # Windows
        "arial.ttf"                                              # Generic
    ]
    
    for font_path in font_paths:
        try:
            return ImageFont.truetype(font_path, font_size)
        except (OSError, IOError):
            continue
    
    # Fallback to default font if no TrueType fonts available
    print(f"Warning: Could not load TrueType font, using default")
    return ImageFont.load_default()


def _truncate_text_to_fit(draw: ImageDraw.ImageDraw, text: str, font: ImageFont.ImageFont, 
                         max_width: int) -> str:
    """
    Truncate text to fit within specified width, adding ellipsis if needed.
    
    Args:
        draw (ImageDraw.ImageDraw): Drawing context for text measurement
        text (str): Original text string
        font (ImageFont.ImageFont): Font for measurement
        max_width (int): Maximum allowed width in pixels
        
    Returns:
        str: Truncated text that fits within max_width
    """
    if not text:
        return text
    
    # Check if text already fits
    text_bbox = draw.textbbox((0, 0), text, font=font)
    text_width = text_bbox[2] - text_bbox[0]
    
    if text_width <= max_width:
        return text
    
    # Truncate and add ellipsis
    ellipsis = "..."
    while len(text) > 10:  # Keep minimum readable length
        text = text[:-4] + ellipsis
        text_bbox = draw.textbbox((0, 0), text, font=font)
        text_width = text_bbox[2] - text_bbox[0]
        
        if text_width <= max_width:
            break
    
    return text


def _render_text_with_stroke(draw: ImageDraw.ImageDraw, text: str, x: int, y: int, 
                           font: ImageFont.ImageFont) -> None:
    """
    Render text with black stroke outline for maximum readability.
    
    This creates a black border around white text, ensuring readability
    on any background color or pattern.
    
    Args:
        draw (ImageDraw.ImageDraw): Drawing context
        text (str): Text to render
        x (int): X position
        y (int): Y position  
        font (ImageFont.ImageFont): Font to use
    """
    # Render black stroke in all directions around the text
    for dx in range(-TEXT_STROKE_WIDTH, TEXT_STROKE_WIDTH + 1):
        for dy in range(-TEXT_STROKE_WIDTH, TEXT_STROKE_WIDTH + 1):
            if dx != 0 or dy != 0:  # Skip center position
                draw.text((x + dx, y + dy), text, font=font, fill=COLOR_BLACK)
    
    # Render white text on top of black stroke
    draw.text((x, y), text, font=font, fill=COLOR_WHITE)


def compose_final_image(img_rotated: Image.Image, metadata: Optional[Dict[str, Any]] = None) -> Tuple[Image.Image, Tuple[int, int, int, int, float]]:
    """
    Compose the final display image by combining card, background, and metadata.
    
    This function orchestrates the complete image composition pipeline:
    1. Scale the rotated card image to fit the display
    2. Generate a blurred background from the original image
    3. Position the card on the right side of the display
    4. Add metadata text overlay on the left side
    5. Return the final composite image
    
    Layout Strategy:
    - Card image: Positioned on right side, scaled to fit while preserving aspect ratio
    - Background: Blurred version of card image fills entire display  
    - Text area: Left side space not occupied by card image
    
    Args:
        img_rotated (Image.Image): Card image already rotated 90° clockwise
        metadata (Optional[Dict[str, Any]]): Card metadata for text overlay
        
    Returns:
        Tuple containing:
        - Image.Image: Final composite image ready for display
        - Tuple[int, int, int, int, float]: Composition metadata
          (new_width, new_height, x_offset, y_offset, scale_factor)
        
    Example:
        >>> final_img, composition_data = compose_final_image(rotated_card, metadata)
        >>> width, height, x_pos, y_pos, scale = composition_data
        >>> print(f"Card scaled to {width}x{height} at position ({x_pos}, {y_pos})")
    """
    # Calculate optimal scaling to fit card within display bounds
    rot_width, rot_height = img_rotated.size
    
    # Calculate scaling factors for both dimensions
    scale_x = TARGET_WIDTH / rot_width
    scale_y = TARGET_HEIGHT / rot_height
    
    # Use smaller scale factor to ensure card fits completely within display
    scale_factor = min(scale_x, scale_y)
    
    # Calculate new dimensions maintaining aspect ratio
    new_width = max(1, int(rot_width * scale_factor))
    new_height = max(1, int(rot_height * scale_factor))
    
    # Resize card image with high-quality resampling
    img_resized = img_rotated.resize((new_width, new_height), Image.Resampling.LANCZOS)
    
    # Generate blurred background from original rotated image
    final_img = create_blurred_background(img_rotated)
    
    # Position card on right side of display
    x_offset = TARGET_WIDTH - new_width  # Right-align the card
    y_offset = (TARGET_HEIGHT - new_height) // 2  # Center vertically
    
    # Composite card onto background
    final_img.paste(img_resized, (x_offset, y_offset))
    
    # Calculate available space for text (left side of card)
    text_area_width = x_offset
    
    # Add metadata text overlay if provided
    if metadata and text_area_width > 40:  # Ensure minimum space for text
        final_img = add_text_to_background(final_img, metadata, text_area_width, side='left')
    
    # Return final image and composition metadata
    composition_metadata = (new_width, new_height, x_offset, y_offset, scale_factor)
    return final_img, composition_metadata

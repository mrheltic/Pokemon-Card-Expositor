#!/usr/bin/env python3
"""
Pokemon Expositor Image Converter
All-in-one tool for converting images to 1024x600 RAW RGB565 f            # Grading info in the margin area
            grading_x = img_width + 10  # Same margin area
            grading_y_start = 120  # Below the card details

            grading_info = [
                "GRADE: PSA 10",
                "POPULATION: 1/1",
                "CERT #: " + card_id.split('-')[0].upper() + "001"
            ]

            for i, info in enumerate(grading_info):
                y_pos = grading_y_start + (i * 20)
                if y_pos < TARGET_HEIGHT - 10:  # Don't go too close to bottom
                    draw.text((grading_x+1, y_pos+1), info, shadow_color, font=font_medium)
                    draw.text((grading_x, y_pos), info, text_color, font=font_medium)approach: works with any image dimensions, auto-calculates margins
Optimized for Pokemon card display with vertical orientation
"""

import os
import sys
import glob
from PIL import Image, ImageFilter, ImageDraw, ImageFont
import struct
import json

# Configuration for Pokemon Expositor
TARGET_WIDTH = 1024
TARGET_HEIGHT = 600
BYTES_PER_PIXEL = 2

def rgb888_to_rgb565(r, g, b):
    # Convert 8-bit RGB to 5-6-5 RGB
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    rgb565 = (r5 << 11) | (g6 << 5) | b5
    return struct.pack('<H', rgb565)  # Little endian uint16

def create_grading_background(img_rotated, card_metadata=None):
    """
    Create a professional grading-style background with blurred image extract.
    """
    # Create base background
    background = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), (15, 15, 15))

    # Create blurred version of the rotated image for background
    # Scale down for performance, then blur, then scale back up
    blur_scale = 0.3  # Scale down to 30% for better blur performance
    small_size = (int(img_rotated.width * blur_scale), int(img_rotated.height * blur_scale))
    img_small = img_rotated.resize(small_size, Image.Resampling.LANCZOS)

    # Apply heavy blur
    img_blurred = img_small.filter(ImageFilter.GaussianBlur(radius=8))

    # Scale back up to target size
    img_blurred_large = img_blurred.resize((TARGET_WIDTH, TARGET_HEIGHT), Image.Resampling.LANCZOS)

    # Blend with dark overlay for professional look
    overlay = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), (0, 0, 0))
    background = Image.blend(img_blurred_large, overlay, 0.7)  # 70% blur, 30% dark overlay

    return background
def convert_image_to_raw(input_path, output_png_path=None, output_raw_path=None, card_metadata=None):
    """
    Convert any image to 1024x600 RAW RGB565 format with professional grading-style margins.
    Creates blurred background from image and overlays professional card data text.
    """
    try:
        print(f"🔄 Processing: {input_path}")
        
        # Load and open the image
        img = Image.open(input_path)
        
        # Convert to RGB if needed
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Rotate 90 degrees first for horizontal display
        img_rotated = img.rotate(90, expand=True)
        
        # Get rotated dimensions
        rot_width, rot_height = img_rotated.size
        
        # Calculate aspect ratio
        aspect_ratio = rot_width / rot_height
        
        # Calculate scaling to fit within target dimensions
        scale_x = TARGET_WIDTH / rot_width
        scale_y = TARGET_HEIGHT / rot_height
        scale_factor = min(scale_x, scale_y)
        
        # Calculate new dimensions after scaling
        new_width = int(rot_width * scale_factor)
        new_height = int(rot_height * scale_factor)
        
        # Resize image maintaining aspect ratio
        img_resized = img_rotated.resize((new_width, new_height), Image.Resampling.LANCZOS)

        # Calculate automatic margin to reach exactly 1024x600
        margin_left = TARGET_WIDTH - new_width
        margin_bottom = TARGET_HEIGHT - new_height
        
        # Create final image with exact target dimensions
        final_img = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), (15, 15, 15))
        
        # Create professional grading-style background from resized image
        # Scale up the resized image to create background
        bg_scale = max(TARGET_WIDTH / new_width, TARGET_HEIGHT / new_height) * 1.2
        bg_width = int(new_width * bg_scale)
        bg_height = int(new_height * bg_scale)
        img_background = img_resized.resize((bg_width, bg_height), Image.Resampling.LANCZOS)
        
        # Apply heavy blur for background
        img_blurred = img_background.filter(ImageFilter.GaussianBlur(radius=8))
        
        # Center the blurred background
        bg_x = (TARGET_WIDTH - bg_width) // 2
        bg_y = (TARGET_HEIGHT - bg_height) // 2
        
        # Paste blurred background
        final_img.paste(img_blurred, (bg_x, bg_y))
        
        # Add dark overlay for professional look
        overlay = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), (0, 0, 0))
        final_img = Image.blend(final_img, overlay, 0.7)

        # Position image on the right (margin on the left)
        x_offset = TARGET_WIDTH - new_width  # Place at right edge
        y_offset = 0  # Place at top edge (margin will be at bottom)

        # Paste main image
        final_img.paste(img_resized, (x_offset, y_offset))

        # Text overlay removed - keeping only clean image with left margin

        # Save PNG version if requested
        if output_png_path:
            final_img.save(output_png_path, 'PNG')
            print(f"   ✅ Saved PNG: {output_png_path}")

        # Determine RAW output path
        if output_raw_path is None:
            name, _ = os.path.splitext(input_path)
            output_raw_path = f"{name}_1024x600.raw"

        # Convert to RAW RGB565 format
        print(f"   🔄 Converting to RGB565...")
        raw_data = bytearray()

        # Process pixel by pixel
        pixels = final_img.load()
        for y in range(TARGET_HEIGHT):
            for x in range(TARGET_WIDTH):
                r, g, b = pixels[x, y]
                rgb565_bytes = rgb888_to_rgb565(r, g, b)
                raw_data.extend(rgb565_bytes)

        # Write RAW file
        with open(output_raw_path, 'wb') as f:
            f.write(raw_data)

        expected_size = TARGET_WIDTH * TARGET_HEIGHT * BYTES_PER_PIXEL
        actual_size = len(raw_data)

        print(f"   ✅ Saved RAW: {output_raw_path}")
        print(f"   📐 Scale: {scale_factor:.2f}x, Aspect: {aspect_ratio:.2f}")
        print(f"   📏 Final: {new_width}×{new_height} → {TARGET_WIDTH}×{TARGET_HEIGHT}")
        print(f"   🎯 Position: ({x_offset}, {y_offset}), Auto Margins: {margin_left}px left, {margin_bottom}px bottom")
        print(f"   💾 Size: {actual_size:,} bytes (expected: {expected_size:,})")

        return output_raw_path

    except Exception as e:
        print(f"   ❌ Error: {e}")
        return None

def main():
    if len(sys.argv) < 2:
        print("📋 Usage:")
        print("  python3 pokemon_converter.py <input_file> [output_png] [output_raw]")
        print("  python3 pokemon_converter.py <input_file> [output_png] [output_raw] [metadata_json]")
        print()
        print("📁 Examples:")
        print("  python3 pokemon_converter.py image.png")
        print("  python3 pokemon_converter.py image.png output.png output.raw")
        print("  python3 pokemon_converter.py image.png output.png output.raw '{\"name\":\"Pikachu\"}'")
        print("  python3 pokemon_converter.py *.jpg *.png")
        print("  python3 pokemon_converter.py /path/to/images/")
        print()
        print("🎯 Features:")
        print("  • Agnostic image processing - works with any input dimensions")
        print("  • Automatic margin calculation for perfect centering")
        print("  • Professional grading-style backgrounds with blurred image extracts")
        print("  • Automatic card information overlay (ID, name, set, year, rarity)")
        print("  • Maintains original aspect ratio")
        print("  • Rotates images 90° left for vertical display")
        print("  • Scales to fit screen while preserving proportions")
        print("  • Optimized for any display device")
        return
    
    # Check if called with specific output paths and optional metadata
    if len(sys.argv) >= 4:
        # Called from server with specific paths
        input_file = sys.argv[1]
        output_png = sys.argv[2]
        output_raw = sys.argv[3]
        
        # Check if metadata JSON is provided
        card_metadata = None
        if len(sys.argv) >= 5:
            try:
                card_metadata = json.loads(sys.argv[4])
                print(f"   📋 Using metadata: {card_metadata.get('name', 'Unknown')}")
            except json.JSONDecodeError as e:
                print(f"   ⚠️  Invalid metadata JSON: {e}")
        
        result = convert_image_to_raw(input_file, output_png, output_raw, card_metadata)
        if result:
            print(f"\n✅ Conversion complete!")
        else:
            print(f"\n❌ Conversion failed!")
    else:
        # Process single file with auto-generated names
        input_file = sys.argv[1]
        result = convert_image_to_raw(input_file)
        if result:
            print(f"\n✅ Conversion complete!")
        else:
            print(f"\n❌ Conversion failed!")

if __name__ == "__main__":
    main()

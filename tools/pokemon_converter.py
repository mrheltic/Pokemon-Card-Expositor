#!/usr/bin/env python3
"""
Pokemon Expositor Image Converter
All-in-one tool for converting images to 1024x600 RAW RGB565 format
Agnostic approach: works with any image dimensions, auto-calculates margins
"""

import os
import sys
import glob
from PIL import Image
import struct

# Configuration for Pokemon Expositor
TARGET_WIDTH = 1024
TARGET_HEIGHT = 600
BYTES_PER_PIXEL = 2  # RGB565

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565 format"""
    # Convert 8-bit RGB to 5-6-5 format
    r5 = (r >> 3) & 0x1F  # 5 bits
    g6 = (g >> 2) & 0x3F  # 6 bits  
    b5 = (b >> 3) & 0x1F  # 5 bits
    
    # Pack into 16-bit value (little endian)
    rgb565 = (r5 << 11) | (g6 << 5) | b5
    return struct.pack('<H', rgb565)  # Little endian uint16

def convert_image_to_raw(input_path, output_png_path=None, output_raw_path=None):
    """
    Convert any image to 1024x600 RAW RGB565 format with automatic margin calculation.
    Agnostic approach: maintains aspect ratio and auto-calculates margins for perfect fit.
    """
    try:
        print(f"🔄 Processing: {input_path}")

        # Open and load image
        with Image.open(input_path) as img:
            # Convert to RGB if needed
            if img.mode in ('RGBA', 'LA', 'P'):
                img = img.convert('RGB')

            # Rotate 90 degrees counterclockwise for vertical display
            img_rotated = img.rotate(90, expand=True)

            print(f"   Original: {img.size} → Rotated: {img_rotated.size}")

            # Get rotated dimensions
            img_width, img_height = img_rotated.size

            # Calculate aspect ratio
            aspect_ratio = img_width / img_height

            # Calculate scaling to fit within target while maintaining aspect ratio
            # We want to fit the entire image, so scale to the smaller constraining dimension
            scale_x = TARGET_WIDTH / img_width
            scale_y = TARGET_HEIGHT / img_height
            scale_factor = min(scale_x, scale_y)

            # Apply scaling
            new_width = int(img_width * scale_factor)
            new_height = int(img_height * scale_factor)

            # Resize image maintaining aspect ratio
            img_resized = img_rotated.resize((new_width, new_height), Image.Resampling.LANCZOS)

            # Create black background at target resolution
            final_img = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), (0, 0, 0))

            # Auto-calculate margins for perfect centering
            # Center horizontally and vertically
            x_margin = (TARGET_WIDTH - new_width) // 2
            y_margin = (TARGET_HEIGHT - new_height) // 2

            # Ensure margins are not negative (shouldn't happen with our scaling)
            x_offset = max(0, x_margin)
            y_offset = max(0, y_margin)

            # Paste resized image onto black background
            final_img.paste(img_resized, (x_offset, y_offset))
            
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
            print(f"   🎯 Position: ({x_offset}, {y_offset}), Margins: H{x_margin}px, V{y_margin}px")
            print(f"   💾 Size: {actual_size:,} bytes (expected: {expected_size:,})")

            return output_raw_path
            
            return output_raw_path
            
    except Exception as e:
        print(f"   ❌ Error: {e}")
        return None

def process_files(file_patterns):
    """Process multiple files or patterns"""
    files_processed = 0
    files_failed = 0
    
    all_files = []
    
    # Expand all patterns and collect files
    for pattern in file_patterns:
        if os.path.isfile(pattern):
            all_files.append(pattern)
        elif os.path.isdir(pattern):
            # Process directory
            image_extensions = ['.jpg', '.jpeg', '.png', '.bmp', '.tiff', '.webp', '.gif']
            for filename in os.listdir(pattern):
                if any(filename.lower().endswith(ext) for ext in image_extensions):
                    all_files.append(os.path.join(pattern, filename))
        else:
            # Try glob pattern
            matches = glob.glob(pattern)
            if matches:
                all_files.extend(matches)
            else:
                print(f"⚠️  No files found matching: {pattern}")
                files_failed += 1
    
    # Remove duplicates
    all_files = list(set(all_files))
    
    # Process each file
    for filepath in sorted(all_files):
        if convert_image_to_raw(filepath):
            files_processed += 1
        else:
            files_failed += 1
    
    return files_processed, files_failed

def main():
    print("🎮 Pokemon Expositor Image Converter")
    print("=====================================")
    
    if len(sys.argv) < 2:
        print("📋 Usage:")
        print("  python3 pokemon_converter.py <input_file> [output_png] [output_raw]")
        print()
        print("📁 Examples:")
        print("  python3 pokemon_converter.py image.png")
        print("  python3 pokemon_converter.py image.png output.png output.raw")
        print("  python3 pokemon_converter.py *.jpg *.png")
        print("  python3 pokemon_converter.py /path/to/images/")
        print()
        print("🎯 Features:")
        print("  • Agnostic image processing - works with any input dimensions")
        print("  • Automatic margin calculation for perfect centering")
        print("  • Maintains original aspect ratio")
        print("  • Rotates images 90° left for vertical display")
        print("  • Scales to fit screen while preserving proportions")
        print("  • Outputs RAW RGB565 format for DMA")
        print("  • Optimized for any display device")
        return
    
    # Check if called with specific output paths
    if len(sys.argv) == 4:
        # Called from server with specific paths
        input_file = sys.argv[1]
        output_png = sys.argv[2]
        output_raw = sys.argv[3]
        
        result = convert_image_to_raw(input_file, output_png, output_raw)
        if result:
            print(f"\n✅ Conversion complete!")
        else:
            print(f"\n❌ Conversion failed!")
        return
    
    # Original batch processing mode
    files_processed, files_failed = process_files(sys.argv[1:])
    
    # Summary
    print(f"\n🎊 Conversion Complete!")
    print(f"   ✅ Successfully processed: {files_processed} files")
    if files_failed > 0:
        print(f"   ❌ Failed: {files_failed} files")
    
    if files_processed > 0:
        print(f"\n📱 RAW files ready for Pokemon Expositor!")
        print(f"   💾 Expected size per file: {TARGET_WIDTH * TARGET_HEIGHT * BYTES_PER_PIXEL:,} bytes")
        print(f"   🚀 Copy *.raw files to SD card root directory")

if __name__ == "__main__":
    main()

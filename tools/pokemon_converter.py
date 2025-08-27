#!/usr/bin/env python3
"""
Pokemon Expositor Image Converter
All-in-one tool for converting images to 1024x600 RAW RGB565 format
Optimized for Pokemon card display with vertical orientation
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

def convert_image_to_raw(input_path, output_path=None):
    """
    Convert image to 1024x600 RAW RGB565 with rotation and letterboxing
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
            
            # Calculate scaling to fit within target size while maintaining aspect ratio
            img_width, img_height = img_rotated.size
            scale_factor = min(TARGET_WIDTH / img_width, TARGET_HEIGHT / img_height)
            
            # Calculate new size
            new_width = int(img_width * scale_factor)
            new_height = int(img_height * scale_factor)
            
            # Resize image maintaining aspect ratio
            img_resized = img_rotated.resize((new_width, new_height), Image.Resampling.LANCZOS)
            
            # Create black background at target resolution
            final_img = Image.new('RGB', (TARGET_WIDTH, TARGET_HEIGHT), (0, 0, 0))
            
            # Calculate position to center the image
            x_offset = (TARGET_WIDTH - new_width) // 2
            y_offset = (TARGET_HEIGHT - new_height) // 2
            
            # Paste resized image onto black background
            final_img.paste(img_resized, (x_offset, y_offset))
            
            # Determine output path
            if output_path is None:
                name, _ = os.path.splitext(input_path)
                output_path = f"{name}_1024x600.raw"
            
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
            with open(output_path, 'wb') as f:
                f.write(raw_data)
            
            expected_size = TARGET_WIDTH * TARGET_HEIGHT * BYTES_PER_PIXEL
            actual_size = len(raw_data)
            
            print(f"   ✅ Saved: {output_path}")
            print(f"   📐 Scale: {scale_factor:.2f}x, Position: ({x_offset}, {y_offset})")
            print(f"   💾 Size: {actual_size:,} bytes (expected: {expected_size:,})")
            
            return output_path
            
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
        print("  python3 pokemon_converter.py <files...>")
        print()
        print("📁 Examples:")
        print("  python3 pokemon_converter.py image.png")
        print("  python3 pokemon_converter.py *.jpg *.png")
        print("  python3 pokemon_converter.py /path/to/images/")
        print("  python3 pokemon_converter.py card1.png card2.jpg folder/")
        print()
        print("🎯 Features:")
        print("  • Rotates images 90° left for vertical display")
        print("  • Resizes to 1024x600 maintaining aspect ratio")
        print("  • Adds black bars on left/right sides")
        print("  • Outputs RAW RGB565 format for DMA")
        print("  • Optimized for Pokemon card display")
        return
    
    # Process all provided files/patterns
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

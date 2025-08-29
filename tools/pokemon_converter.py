#!/usr/bin/env python3
"""
Pokemon Card Expositor - Main Conversion Script

This is the primary command-line interface for converting Pokemon card images
into display-ready format for the 5-inch ESP32 display system. The script
orchestrates the complete conversion pipeline from source image to RGB565
binary format.

Technical Pipeline:
1. Load source image and validate format
2. Apply 90° rotation for display orientation compensation
3. Generate blurred background and scale card image
4. Overlay metadata text with stroke effects
5. Export as PNG preview and RGB565 binary for ESP32

Usage Examples:
    # Basic conversion
    python3 pokemon_converter.py input.png output.png output.raw
    
    # With metadata
    python3 pokemon_converter.py input.png output.png output.raw '{"name":"Pikachu","set_name":"Base Set"}'
    
    # PNG only
    python3 pokemon_converter.py input.png output.png None
    
    # RAW only  
    python3 pokemon_converter.py input.png None output.raw

Exit Codes:
    0: Success - conversion completed without errors
    1: Usage error - invalid command line arguments
    2: Conversion error - processing failed due to image or file issues

Author: mrheltic
Date: August 2025
"""

import sys
import json
import os
import argparse
from typing import Dict, Any, Optional

from pc_convert import convert_single, validate_conversion_setup
from pc_utils import ensure_dir, validate_card_id, sanitize_filename
from pc_io import download_card_image


def parse_arguments() -> argparse.Namespace:
    """
    Parse command line arguments with comprehensive validation.
    
    Returns:
        argparse.Namespace: Parsed arguments with validation
    """
    parser = argparse.ArgumentParser(
        description='Convert Pokemon card images for 5-inch display system',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  # Convert downloaded image with automatic metadata
  %(prog)s sv10-193
  
  # Convert local image file
  %(prog)s input.png output.png output.raw
  
  # Convert with custom metadata
  %(prog)s input.png output.png output.raw '{"name":"Custom Name"}'
  
  # PNG preview only
  %(prog)s input.png output.png None
        '''
    )
    
    parser.add_argument('input', 
                       help='Input image file path or Pokemon card ID (e.g., sv10-193)')
    
    parser.add_argument('output_png', nargs='?',
                       help='Output PNG file path (None to skip)')
    
    parser.add_argument('output_raw', nargs='?', 
                       help='Output RGB565 binary file path (None to skip)')
    
    parser.add_argument('metadata', nargs='?',
                       help='JSON metadata string for card information')
    
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose output for debugging')
    
    parser.add_argument('--validate', action='store_true',
                       help='Validate conversion setup and exit')
    
    parser.add_argument('--force', '-f', action='store_true',
                       help='Overwrite existing output files')
    
    return parser.parse_args()


def parse_metadata(metadata_str: Optional[str]) -> Optional[Dict[str, Any]]:
    """
    Parse metadata JSON string with error handling.
    
    Args:
        metadata_str (Optional[str]): JSON string containing card metadata
        
    Returns:
        Optional[Dict[str, Any]]: Parsed metadata or None if invalid
    """
    if not metadata_str or metadata_str.lower() in ('none', 'null', ''):
        return None
    
    try:
        metadata = json.loads(metadata_str)
        if isinstance(metadata, dict):
            print(f"[INFO] Using metadata: {metadata.get('name', 'Unknown Card')}")
            return metadata
        else:
            print("[WARN] Metadata must be a JSON object")
            return None
    except json.JSONDecodeError as e:
        print(f"[WARN] Invalid metadata JSON: {e}")
        return None


def handle_card_id_input(card_id: str, args: argparse.Namespace) -> tuple:
    """
    Handle Pokemon card ID input by downloading the image and metadata.
    
    Args:
        card_id (str): Pokemon card identifier (e.g., sv10-193)
        args (argparse.Namespace): Command line arguments
        
    Returns:
        tuple: (input_path, output_png, output_raw, metadata)
    """
    if not validate_card_id(card_id):
        raise ValueError(f"Invalid card ID format: {card_id}")
    
    print(f"🎴 Downloading Pokemon card: {card_id}")
    
    try:
        # Download image and get metadata from Pokemon TCG API
        input_path, metadata = download_card_image(card_id)
        
        # Generate output paths if not provided
        safe_name = sanitize_filename(metadata.get('name', card_id))
        base_name = f"{card_id}_{safe_name}"
        
        output_png = args.output_png
        if not output_png or output_png.lower() == 'none':
            output_png = f"images/converted/{base_name}_converted.png"
        
        output_raw = args.output_raw  
        if not output_raw or output_raw.lower() == 'none':
            output_raw = f"images/raw/{base_name}_1024x600.raw"
        
        print(f"📥 Downloaded: {os.path.basename(input_path)}")
        return input_path, output_png, output_raw, metadata
        
    except Exception as e:
        raise RuntimeError(f"Failed to download card {card_id}: {e}")


def check_output_files(output_png: Optional[str], output_raw: Optional[str], force: bool) -> None:
    """
    Check if output files exist and handle overwrite logic.
    
    Args:
        output_png (Optional[str]): PNG output path
        output_raw (Optional[str]): RAW output path  
        force (bool): Whether to force overwrite existing files
        
    Raises:
        FileExistsError: If files exist and force is False
    """
    existing_files = []
    
    if output_png and output_png.lower() != 'none' and os.path.exists(output_png):
        existing_files.append(output_png)
    
    if output_raw and output_raw.lower() != 'none' and os.path.exists(output_raw):
        existing_files.append(output_raw)
    
    if existing_files and not force:
        file_list = '\n  '.join(existing_files)
        raise FileExistsError(f"Output files already exist:\n  {file_list}\nUse --force to overwrite")


def main() -> int:
    """
    Main conversion function with comprehensive error handling.
    
    Returns:
        int: Exit code (0=success, 1=usage error, 2=conversion error)
    """
    try:
        # Parse command line arguments
        args = parse_arguments()
        
        # Handle validation mode
        if args.validate:
            print("[INFO] Validating conversion setup...")
            if validate_conversion_setup():
                print("[SUCCESS] Conversion setup is valid")
                return 0
            else:
                print("[ERROR] Conversion setup validation failed")
                return 2
        
        # Validate minimum arguments
        if not args.input:
            print("[ERROR] Error: Input file or card ID is required")
            print("Use --help for usage information")
            return 1
        
        input_path = args.input
        output_png = args.output_png
        output_raw = args.output_raw
        metadata = parse_metadata(args.metadata)
        
        # Check if input is a Pokemon card ID vs local file
        if validate_card_id(args.input):
            # Handle Pokemon card ID download
            input_path, output_png, output_raw, metadata = handle_card_id_input(args.input, args)
        else:
            # Handle local file input
            if not os.path.exists(input_path):
                print(f"[ERROR] Error: Input file not found: {input_path}")
                return 2
        
        # Normalize None strings to actual None
        if output_png and output_png.lower() in ('none', 'null'):
            output_png = None
        if output_raw and output_raw.lower() in ('none', 'null'):
            output_raw = None
        
        # Validate at least one output format is requested
        if not output_png and not output_raw:
            print("[ERROR] Error: At least one output format (PNG or RAW) must be specified")
            return 1
        
        # Check for existing output files
        try:
            check_output_files(output_png, output_raw, args.force)
        except FileExistsError as e:
            print(f"[ERROR] Error: {e}")
            return 1
        
        # Ensure output directories exist
        if output_png:
            ensure_dir(os.path.dirname(output_png))
        if output_raw:
            ensure_dir(os.path.dirname(output_raw))
        
        # Perform the conversion
        print(f"[INFO] Converting: {os.path.basename(input_path)}")
        
        if args.verbose:
            print(f"   [INFO] Input: {input_path}")
            if output_png:
                print(f"   [INFO] PNG Output: {output_png}")
            if output_raw:
                print(f"   [INFO] RAW Output: {output_raw}")
            if metadata:
                print(f"   [INFO] Metadata: {metadata}")
        
        # Execute conversion pipeline
        final_img, composition_meta = convert_single(input_path, output_png, output_raw, metadata)
        
        # Report success
        print(f"[SUCCESS] Conversion completed successfully")
        if output_png:
            print(f"   [INFO] PNG saved: {os.path.basename(output_png)}")
        if output_raw:
            print(f"   [INFO] RAW saved: {os.path.basename(output_raw)}")
        
        if args.verbose:
            card_w, card_h, card_x, card_y, scale = composition_meta
            print(f"   [INFO] Card: {card_w}x{card_h} at ({card_x}, {card_y}), scale: {scale:.2f}")
        
        return 0
        
    except KeyboardInterrupt:
        print("\n[WARN] Conversion interrupted by user")
        return 1
        
    except Exception as e:
        print(f"[ERROR] Conversion failed: {e}")
        if args.verbose if 'args' in locals() else False:
            import traceback
            print("\n[DEBUG] Detailed error trace:")
            traceback.print_exc()
        return 2


if __name__ == '__main__':
    sys.exit(main())

"""
Pokemon Card Expositor - Conversion Orchestrator

This module provides the high-level conversion workflow that coordinates all
processing steps to transform a Pokemon card image from download to display-ready
format. It serves as the main orchestrator between image processing, I/O operations,
and metadata handling.

Key Functions:
- Single image conversion pipeline
- Format validation and error handling
- Metadata integration and preservation
- Output generation in multiple formats (PNG, RGB565)

Author: mrheltic
Date: August 2025
"""

import os
from typing import Tuple, Optional, Dict, Any
from PIL import Image

from pc_imaging import compose_final_image
from pc_io import save_png, save_raw_rgb565
from pc_constants import TARGET_WIDTH, TARGET_HEIGHT
from pc_utils import validate_image_dimensions


def convert_single(input_path: str, output_png: Optional[str] = None, 
                  output_raw: Optional[str] = None, metadata: Optional[Dict[str, Any]] = None) -> Tuple[Image.Image, Tuple[int, int, int, int, float]]:
    """
    Convert a single Pokemon card image through the complete processing pipeline.
    
    This function orchestrates the entire conversion workflow:
    1. Load and validate the source image
    2. Apply 90° clockwise rotation for display orientation
    3. Compose final image with background and metadata overlay
    4. Save in requested output formats (PNG and/or RGB565)
    5. Return processed image and composition metadata
    
    The rotation step compensates for the physical display mounting which is
    rotated 90° counterclockwise. By rotating the image 90° clockwise in software,
    the final result appears correctly oriented on the rotated display.
    
    Args:
        input_path (str): Path to source image file
        output_png (Optional[str]): Path for PNG output (None to skip)
        output_raw (Optional[str]): Path for RGB565 binary output (None to skip)
        metadata (Optional[Dict[str, Any]]): Card metadata for text overlay
        
    Returns:
        Tuple containing:
        - Image.Image: Final processed image
        - Tuple[int, int, int, int, float]: Composition metadata
          (card_width, card_height, x_position, y_position, scale_factor)
        
    Raises:
        FileNotFoundError: If input_path doesn't exist
        ValueError: If image format is unsupported or dimensions invalid
        IOError: If file operations fail
        
    Example:
        >>> metadata = {'name': 'Pikachu', 'set_name': 'Base Set', 'rarity': 'Common'}
        >>> final_img, composition = convert_single(
        ...     'input.png', 
        ...     'output.png', 
        ...     'output.raw',
        ...     metadata
        ... )
        >>> print(f"Card positioned at {composition[2]}, {composition[3]}")
    """
    # Validate input file exists
    if not os.path.exists(input_path):
        raise FileNotFoundError(f"Input image not found: {input_path}")
    
    try:
        # Load source image with PIL
        print(f"Loading image: {os.path.basename(input_path)}")
        img = Image.open(input_path)
        
        # Validate image properties
        if not validate_image_dimensions(img.width, img.height):
            raise ValueError(f"Invalid image dimensions: {img.width}x{img.height}")
        
        # Ensure RGB color mode for consistent processing
        if img.mode != 'RGB':
            print(f"Converting from {img.mode} to RGB mode")
            img = img.convert('RGB')
        
        # Apply 90° clockwise rotation to compensate for display mounting
        # The physical display is mounted 90° counterclockwise, so we pre-rotate
        # the image 90° clockwise so it appears upright after display rotation
        print("Applying 90° rotation compensation for display orientation")
        img_rotated = img.rotate(90, expand=True)
        
        # Compose final image with background, scaling, and metadata overlay
        print("Composing final image with background and metadata")
        final_img, composition_metadata = compose_final_image(img_rotated, metadata)
        
        # Validate final image dimensions match target display
        if final_img.size != (TARGET_WIDTH, TARGET_HEIGHT):
            raise ValueError(f"Final image size mismatch: {final_img.size} != {TARGET_WIDTH}x{TARGET_HEIGHT}")
        
        # Save PNG format if requested
        if output_png:
            print(f"Saving PNG: {os.path.basename(output_png)}")
            save_png(final_img, output_png, metadata)
        
        # Save RGB565 binary format if requested
        if output_raw:
            print(f"Saving RGB565: {os.path.basename(output_raw)}")
            save_raw_rgb565(final_img, output_raw)
        
        print("Conversion completed successfully")
        return final_img, composition_metadata
        
    except Exception as e:
        # Provide detailed error context for debugging
        error_msg = f"Conversion failed for {input_path}: {str(e)}"
        print(f"Error: {error_msg}")
        raise type(e)(error_msg) from e


def convert_batch(image_list: list, output_dir: str, metadata_list: Optional[list] = None) -> list:
    """
    Convert multiple images in batch with progress tracking.
    
    This function processes multiple Pokemon card images efficiently,
    providing progress feedback and error handling for large conversion jobs.
    
    Args:
        image_list (list): List of input image paths
        output_dir (str): Output directory for processed images
        metadata_list (Optional[list]): List of metadata dicts (same order as images)
        
    Returns:
        list: List of tuples (success: bool, output_path: str, error: str)
        
    Example:
        >>> results = convert_batch(
        ...     ['card1.png', 'card2.png'],
        ...     'output/',
        ...     [{'name': 'Pikachu'}, {'name': 'Charizard'}]
        ... )
        >>> successful = [r for r in results if r[0]]
        >>> print(f"Successfully converted {len(successful)} images")
    """
    from pc_utils import ensure_dir
    
    # Ensure output directory exists
    ensure_dir(output_dir)
    
    results = []
    total_images = len(image_list)
    
    print(f"Starting batch conversion of {total_images} images")
    
    for i, input_path in enumerate(image_list):
        try:
            # Get corresponding metadata if available
            metadata = metadata_list[i] if metadata_list and i < len(metadata_list) else None
            
            # Generate output paths
            base_name = os.path.splitext(os.path.basename(input_path))[0]
            output_png = os.path.join(output_dir, f"{base_name}_converted.png")
            output_raw = os.path.join(output_dir, f"{base_name}_1024x600.raw")
            
            # Convert single image
            print(f"Converting {i+1}/{total_images}: {os.path.basename(input_path)}")
            convert_single(input_path, output_png, output_raw, metadata)
            
            results.append((True, output_png, ""))
            
        except Exception as e:
            error_msg = str(e)
            print(f"Failed to convert {input_path}: {error_msg}")
            results.append((False, "", error_msg))
    
    # Summary
    successful = sum(1 for r in results if r[0])
    print(f"Batch conversion completed: {successful}/{total_images} successful")
    
    return results


def validate_conversion_setup() -> bool:
    """
    Validate that all dependencies and configurations are properly set up.
    
    Returns:
        bool: True if setup is valid, False otherwise
    """
    try:
        # Test PIL functionality
        test_img = Image.new('RGB', (100, 100), (255, 0, 0))
        test_rotated = test_img.rotate(90, expand=True)
        
        # Test processing pipeline components
        from pc_imaging import create_blurred_background
        test_background = create_blurred_background(test_rotated)
        
        print("Conversion setup validation passed")
        return True
        
    except Exception as e:
        print(f"Conversion setup validation failed: {e}")
        return False

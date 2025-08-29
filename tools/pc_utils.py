"""
Pokemon Card Expositor - Utility Functions

This module provides essential utility functions used throughout the Pokemon card
conversion pipeline. It includes color format conversions, file system operations,
validation helpers, and other common functionality.

Author: mrheltic
Date: August 2025
"""

import struct
import os
import re
from typing import Tuple, Optional, Any
from pc_constants import TARGET_WIDTH, TARGET_HEIGHT, BYTES_PER_PIXEL


def rgb888_to_rgb565_bytes(r: int, g: int, b: int) -> bytes:
    """
    Convert 24-bit RGB color to 16-bit RGB565 format in little-endian byte order.
    
    This function is essential for ESP32 display compatibility, as the hardware
    expects RGB565 format for optimal performance and memory usage.
    
    Args:
        r (int): Red component (0-255)
        g (int): Green component (0-255) 
        b (int): Blue component (0-255)
        
    Returns:
        bytes: 2-byte RGB565 representation in little-endian format
        
    Example:
        >>> rgb888_to_rgb565_bytes(255, 0, 0)  # Pure red
        b'\\x00\\xf8'
        >>> rgb888_to_rgb565_bytes(0, 255, 0)  # Pure green
        b'\\xe0\\x07'
    """
    # Convert 8-bit components to reduced bit depths:
    # Red: 8 bits → 5 bits (lose 3 LSBs)
    # Green: 8 bits → 6 bits (lose 2 LSBs) 
    # Blue: 8 bits → 5 bits (lose 3 LSBs)
    r5 = (r >> 3) & 0x1F  # 5-bit red component
    g6 = (g >> 2) & 0x3F  # 6-bit green component  
    b5 = (b >> 3) & 0x1F  # 5-bit blue component
    
    # Pack into 16-bit RGB565 format: RRRRRGGGGGGBBBBB
    rgb565 = (r5 << 11) | (g6 << 5) | b5
    
    # Return as little-endian bytes for ESP32 compatibility
    return struct.pack('<H', rgb565)


def rgb565_to_rgb888(rgb565_bytes: bytes) -> Tuple[int, int, int]:
    """
    Convert 16-bit RGB565 bytes back to 24-bit RGB components.
    
    Useful for debugging and validation of RGB565 conversions.
    
    Args:
        rgb565_bytes (bytes): 2-byte RGB565 data in little-endian format
        
    Returns:
        Tuple[int, int, int]: RGB components (r, g, b) in 0-255 range
    """
    # Unpack little-endian 16-bit value
    rgb565 = struct.unpack('<H', rgb565_bytes)[0]
    
    # Extract components and scale back to 8-bit
    r5 = (rgb565 >> 11) & 0x1F
    g6 = (rgb565 >> 5) & 0x3F
    b5 = rgb565 & 0x1F
    
    # Scale components back to 8-bit range
    r = (r5 << 3) | (r5 >> 2)  # Duplicate MSBs to fill 8 bits
    g = (g6 << 2) | (g6 >> 4)  # Duplicate MSBs to fill 8 bits
    b = (b5 << 3) | (b5 >> 2)  # Duplicate MSBs to fill 8 bits
    
    return r, g, b


def ensure_dir(path: str) -> None:
    """
    Create directory path if it doesn't exist, including parent directories.
    
    This function provides safe directory creation with proper error handling
    and support for nested directory structures.
    
    Args:
        path (str): Directory path to create
        
    Raises:
        OSError: If directory creation fails due to permissions or disk space
        
    Example:
        >>> ensure_dir("images/converted/pokemon")
        # Creates full directory tree if it doesn't exist
    """
    if not path:
        return
        
    try:
        os.makedirs(path, exist_ok=True)
    except OSError as e:
        raise OSError(f"Failed to create directory '{path}': {e}")


def validate_card_id(card_id: str) -> bool:
    """
    Validate Pokemon card ID format.
    
    Valid formats:
    - set-number (e.g., "sv10-193", "base1-4")
    - set abbreviations with numbers and letters
    
    Args:
        card_id (str): Card identifier to validate
        
    Returns:
        bool: True if valid format, False otherwise
        
    Example:
        >>> validate_card_id("sv10-193")
        True
        >>> validate_card_id("invalid_format") 
        False
    """
    if not card_id or not isinstance(card_id, str):
        return False
        
    # Pattern: letters/numbers, dash, numbers/letters
    pattern = r'^[a-zA-Z0-9]+[-][a-zA-Z0-9]+$'
    return bool(re.match(pattern, card_id.strip()))


def sanitize_filename(filename: str) -> str:
    """
    Sanitize filename by removing or replacing invalid characters.
    
    Ensures filenames are safe for all operating systems by removing
    characters that could cause issues in file paths.
    
    Args:
        filename (str): Original filename
        
    Returns:
        str: Sanitized filename safe for file system use
        
    Example:
        >>> sanitize_filename("Misty's Psyduck: Water Type!")
        "Mistys_Psyduck_Water_Type"
    """
    if not filename:
        return "unnamed"
        
    # Remove or replace problematic characters
    sanitized = re.sub(r'[<>:"/\\|?*]', '', filename)  # Remove invalid chars
    sanitized = re.sub(r'[\'"]', '', sanitized)        # Remove quotes
    sanitized = re.sub(r'\s+', '_', sanitized)         # Replace spaces with underscores
    sanitized = re.sub(r'[^\w\-_.]', '', sanitized)    # Keep only alphanumeric, dash, underscore, dot
    
    # Ensure reasonable length
    return sanitized[:100] if len(sanitized) > 100 else sanitized


def calculate_file_size(width: int, height: int, bytes_per_pixel: int = BYTES_PER_PIXEL) -> int:
    """
    Calculate expected file size for raw image data.
    
    Args:
        width (int): Image width in pixels
        height (int): Image height in pixels  
        bytes_per_pixel (int): Bytes per pixel (default: 2 for RGB565)
        
    Returns:
        int: Expected file size in bytes
        
    Example:
        >>> calculate_file_size(1024, 600, 2)
        1228800  # Bytes for 1024x600 RGB565 image
    """
    return width * height * bytes_per_pixel


def validate_image_dimensions(width: int, height: int) -> bool:
    """
    Validate that image dimensions are suitable for processing.
    
    Args:
        width (int): Image width in pixels
        height (int): Image height in pixels
        
    Returns:
        bool: True if dimensions are valid, False otherwise
    """
    # Check for reasonable dimensions
    if width <= 0 or height <= 0:
        return False
    if width > 8192 or height > 8192:  # Prevent excessive memory usage
        return False
    if width < 100 or height < 100:    # Ensure minimum quality
        return False
        
    return True


def format_bytes(size_bytes: int) -> str:
    """
    Format byte size into human-readable string.
    
    Args:
        size_bytes (int): Size in bytes
        
    Returns:
        str: Formatted size string (e.g., "1.2 MB", "500 KB")
    """
    if size_bytes == 0:
        return "0 B"
        
    size_names = ["B", "KB", "MB", "GB"]
    size_index = 0
    size = float(size_bytes)
    
    while size >= 1024.0 and size_index < len(size_names) - 1:
        size /= 1024.0
        size_index += 1
        
    return f"{size:.1f} {size_names[size_index]}"


def get_expected_raw_size() -> int:
    """
    Get the expected size for a raw image file based on target dimensions.
    
    Returns:
        int: Expected file size in bytes for target display resolution
    """
    return calculate_file_size(TARGET_WIDTH, TARGET_HEIGHT, BYTES_PER_PIXEL)

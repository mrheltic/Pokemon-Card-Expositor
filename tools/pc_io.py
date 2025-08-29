"""
Pokemon Card Expositor - Input/Output Operations

This module handles all file I/O operations for the Pokemon card conversion pipeline,
including image downloads from the Pokemon TCG API, file format conversions, and
storage management for different image formats.

Author: mrheltic  
Date: August 2025
"""

import os
import requests
import json
import time
from typing import Optional, Dict, Any, Tuple
from PIL import Image

from pc_constants import (
    TARGET_WIDTH, TARGET_HEIGHT, BYTES_PER_PIXEL,
    POKEMON_TCG_API_BASE, MAX_RETRY_ATTEMPTS, RETRY_DELAY, 
    TIMEOUT_SECONDS, DIR_DOWNLOADED, DIR_CONVERTED, DIR_RAW
)
from pc_utils import rgb888_to_rgb565_bytes, ensure_dir, validate_card_id, sanitize_filename


def download_card_image(card_id: str, card_name: Optional[str] = None) -> Tuple[str, Dict[str, Any]]:
    """
    Download Pokemon card image from the Pokemon TCG API.
    
    This function fetches high-resolution card images and metadata from the official
    Pokemon Trading Card Game API. It includes robust error handling, retry logic,
    and automatic filename generation based on card information.
    
    Args:
        card_id (str): Pokemon card identifier (e.g., "sv10-193", "base1-4")
        card_name (Optional[str]): Override card name for filename generation
        
    Returns:
        Tuple[str, Dict[str, Any]]: Path to downloaded image and card metadata
        
    Raises:
        ValueError: If card_id format is invalid
        requests.RequestException: If download fails after retries
        FileNotFoundError: If API returns no results for the card ID
        
    Example:
        >>> path, metadata = download_card_image("sv10-193")
        >>> print(f"Downloaded: {path}")
        Downloaded: images/downloaded/sv10-193_Mistys_Psyduck.png
    """
    # Validate input
    if not validate_card_id(card_id):
        raise ValueError(f"Invalid card ID format: {card_id}")
    
    # Ensure download directory exists
    download_dir = os.path.join("images", DIR_DOWNLOADED)
    ensure_dir(download_dir)
    
    # Query Pokemon TCG API for card information
    print(f"Searching for card: {card_id}")
    api_url = f"{POKEMON_TCG_API_BASE}?q=id:{card_id}"
    
    # Download with retry logic
    for attempt in range(MAX_RETRY_ATTEMPTS):
        try:
            response = requests.get(api_url, timeout=TIMEOUT_SECONDS)
            response.raise_for_status()
            
            data = response.json()
            if not data.get('data'):
                raise FileNotFoundError(f"No card found with ID: {card_id}")
            
            card_data = data['data'][0]
            break
            
        except requests.RequestException as e:
            if attempt == MAX_RETRY_ATTEMPTS - 1:
                raise requests.RequestException(f"Failed to fetch card data after {MAX_RETRY_ATTEMPTS} attempts: {e}")
            
            print(f"API request failed (attempt {attempt + 1}/{MAX_RETRY_ATTEMPTS}): {e}")
            time.sleep(RETRY_DELAY * (2 ** attempt))  # Exponential backoff
    
    # Extract card metadata
    metadata = {
        'id': card_data.get('id', card_id),
        'name': card_name or card_data.get('name', 'Unknown Card'),
        'set_name': card_data.get('set', {}).get('name', 'Unknown Set'),
        'set_series': card_data.get('set', {}).get('series', ''),
        'release_date': card_data.get('set', {}).get('releaseDate', ''),
        'rarity': card_data.get('rarity', ''),
        'artist': card_data.get('artist', ''),
        'hp': card_data.get('hp', ''),
        'types': card_data.get('types', [])
    }
    
    # Get high-resolution image URL
    image_url = card_data.get('images', {}).get('large')
    if not image_url:
        raise FileNotFoundError(f"No high-resolution image available for card: {card_id}")
    
    # Generate safe filename
    safe_name = sanitize_filename(metadata['name'])
    filename = f"{card_id}_{safe_name}.png"
    image_path = os.path.join(download_dir, filename)
    
    # Skip download if file already exists and is valid
    if os.path.exists(image_path):
        try:
            # Verify existing file is valid
            with Image.open(image_path) as img:
                img.verify()
            print(f"Using existing image: {filename}")
            return image_path, metadata
        except Exception:
            print(f"Existing image corrupted, re-downloading: {filename}")
    
    # Download the image with retry logic
    print(f"Downloading image: {filename}")
    for attempt in range(MAX_RETRY_ATTEMPTS):
        try:
            img_response = requests.get(image_url, timeout=TIMEOUT_SECONDS)
            img_response.raise_for_status()
            
            # Save image data
            with open(image_path, 'wb') as f:
                f.write(img_response.content)
            
            # Verify downloaded image
            with Image.open(image_path) as img:
                img.verify()
            
            print(f"Successfully downloaded: {filename}")
            return image_path, metadata
            
        except (requests.RequestException, IOError) as e:
            if attempt == MAX_RETRY_ATTEMPTS - 1:
                raise requests.RequestException(f"Failed to download image after {MAX_RETRY_ATTEMPTS} attempts: {e}")
            
            print(f"Image download failed (attempt {attempt + 1}/{MAX_RETRY_ATTEMPTS}): {e}")
            time.sleep(RETRY_DELAY * (2 ** attempt))


def save_png(image: Image.Image, path: str, metadata: Optional[Dict[str, Any]] = None) -> None:
    """
    Save PIL image as PNG with optional metadata preservation.
    
    This function saves processed images in PNG format with optimized compression
    settings and embedded metadata for debugging and validation purposes.
    
    Args:
        image (Image.Image): PIL Image object to save
        path (str): Output file path
        metadata (Optional[Dict[str, Any]]): Card metadata to embed in PNG
        
    Raises:
        IOError: If file cannot be saved
        
    Example:
        >>> save_png(processed_image, "output.png", card_metadata)
    """
    # Ensure output directory exists
    ensure_dir(os.path.dirname(path))
    
    try:
        # Prepare PNG metadata
        pnginfo = None
        if metadata:
            from PIL.PngImagePlugin import PngInfo
            pnginfo = PngInfo()
            
            # Add metadata as PNG text chunks
            for key, value in metadata.items():
                if value:  # Only add non-empty values
                    pnginfo.add_text(f"pokemon_{key}", str(value))
        
        # Save with optimized settings
        image.save(path, 'PNG', pnginfo=pnginfo, optimize=True, compress_level=6)
        print(f"Saved PNG: {os.path.basename(path)}")
        
    except IOError as e:
        raise IOError(f"Failed to save PNG to {path}: {e}")


def save_raw_rgb565(image: Image.Image, path: str) -> None:
    """
    Convert PIL image to RGB565 binary format for ESP32 display.
    
    This function converts a 24-bit RGB PIL image to 16-bit RGB565 binary format
    optimized for ESP32 display hardware. The output is stored in row-major order
    with little-endian byte ordering for direct hardware compatibility.
    
    The RGB565 format reduces memory usage by 33% compared to RGB888 while
    maintaining acceptable visual quality for display applications.
    
    Args:
        image (Image.Image): PIL Image in RGB mode with TARGET_WIDTH x TARGET_HEIGHT dimensions
        path (str): Output file path for binary data
        
    Raises:
        ValueError: If image dimensions don't match target resolution
        IOError: If file cannot be written
        
    Example:
        >>> save_raw_rgb565(processed_image, "card.raw")
        # Creates 1,228,800 byte file for 1024x600 display
    """
    # Validate image format and dimensions
    if image.mode != 'RGB':
        raise ValueError(f"Image must be in RGB mode, got: {image.mode}")
    
    if image.size != (TARGET_WIDTH, TARGET_HEIGHT):
        raise ValueError(f"Image dimensions must be {TARGET_WIDTH}x{TARGET_HEIGHT}, got: {image.size}")
    
    # Ensure output directory exists
    ensure_dir(os.path.dirname(path))
    
    try:
        # Convert to RGB565 binary format
        pixels = image.load()
        with open(path, 'wb') as f:
            bytes_written = 0
            
            # Process pixels in row-major order (y, then x)
            for y in range(TARGET_HEIGHT):
                for x in range(TARGET_WIDTH):
                    r, g, b = pixels[x, y]
                    
                    # Convert RGB888 to RGB565 and write as little-endian bytes
                    rgb565_bytes = rgb888_to_rgb565_bytes(r, g, b)
                    f.write(rgb565_bytes)
                    bytes_written += BYTES_PER_PIXEL
        
        # Verify file size
        expected_size = TARGET_WIDTH * TARGET_HEIGHT * BYTES_PER_PIXEL
        if bytes_written != expected_size:
            raise IOError(f"RGB565 file size mismatch: wrote {bytes_written}, expected {expected_size}")
        
        print(f"Saved RGB565: {os.path.basename(path)} ({bytes_written:,} bytes)")
        
    except IOError as e:
        raise IOError(f"Failed to save RGB565 to {path}: {e}")


def load_batch_file(file_path: str) -> list:
    """
    Load batch processing configuration from JSON file.
    
    Args:
        file_path (str): Path to JSON configuration file
        
    Returns:
        list: List of card dictionaries with 'id' and optional 'name' fields
        
    Raises:
        FileNotFoundError: If file doesn't exist
        json.JSONDecodeError: If file contains invalid JSON
        
    Example JSON format:
        [
            {"id": "sv10-193", "name": "Misty's Psyduck"},
            {"id": "base1-4"},
            {"id": "xy1-1", "name": "Custom Name"}
        ]
    """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        # Validate data format
        if not isinstance(data, list):
            raise ValueError("JSON file must contain an array of card objects")
        
        # Validate each card entry
        validated_cards = []
        for i, card in enumerate(data):
            if not isinstance(card, dict) or 'id' not in card:
                raise ValueError(f"Card {i} must be an object with 'id' field")
            
            if not validate_card_id(card['id']):
                raise ValueError(f"Invalid card ID format at index {i}: {card['id']}")
            
            validated_cards.append({
                'id': card['id'],
                'name': card.get('name')
            })
        
        return validated_cards
        
    except FileNotFoundError:
        raise FileNotFoundError(f"Batch file not found: {file_path}")
    except json.JSONDecodeError as e:
        raise json.JSONDecodeError(f"Invalid JSON in {file_path}: {e}")


def cleanup_temp_files(directory: str, max_age_hours: int = 24) -> int:
    """
    Clean up temporary files older than specified age.
    
    Args:
        directory (str): Directory to clean
        max_age_hours (int): Maximum file age in hours
        
    Returns:
        int: Number of files removed
    """
    if not os.path.exists(directory):
        return 0
    
    import time
    current_time = time.time()
    max_age_seconds = max_age_hours * 3600
    files_removed = 0
    
    for filename in os.listdir(directory):
        file_path = os.path.join(directory, filename)
        if os.path.isfile(file_path):
            file_age = current_time - os.path.getmtime(file_path)
            if file_age > max_age_seconds:
                try:
                    os.remove(file_path)
                    files_removed += 1
                except OSError:
                    pass  # Continue if file cannot be removed
    
    return files_removed

"""
Pokemon Card Expositor - System Constants

This module defines all constants used throughout the Pokemon card conversion pipeline.
These values are optimized for a 5-inch display with 1024x600 resolution mounted in
portrait orientation (rotated 90° left from landscape).

Author: mrheltic
Date: August 2025
"""

# Display Configuration
# These constants define the target display specifications
TARGET_WIDTH = 1024      # Display width in pixels (after rotation)
TARGET_HEIGHT = 600      # Display height in pixels (after rotation)
BYTES_PER_PIXEL = 2      # RGB565 format uses 2 bytes per pixel

# Pokemon TCG API Configuration
# Base URL for the Pokemon Trading Card Game API
POKEMON_TCG_API_BASE = "https://api.pokemontcg.io/v2/cards"

# Image Processing Parameters
# These values control the visual quality and performance of image processing
BLUR_RADIUS = 8                    # Gaussian blur radius for background effect (pixels)
BLUR_SCALE_FACTOR = 0.3           # Scale factor for blur performance optimization
BACKGROUND_OPACITY = 0.7          # Background opacity for text contrast (0.0-1.0)

# Text Rendering Configuration
# These parameters control the appearance of metadata text overlay
MAX_FONT_SIZE = 38                # Maximum font size for card metadata text (pixels)
MIN_FONT_SIZE = 12                # Minimum font size before text truncation (pixels)
TEXT_STROKE_WIDTH = 2             # Black stroke width around text for readability (pixels)
LINE_SPACING = 4                  # Additional spacing between text lines (pixels)

# Color Definitions (RGB format)
# Standard colors used throughout the processing pipeline
COLOR_WHITE = (255, 255, 255)     # Pure white for text fill
COLOR_BLACK = (0, 0, 0)           # Pure black for text stroke
COLOR_TRANSPARENT = (0, 0, 0, 0)  # Transparent color for overlay creation

# File System Configuration
# Directory structure for organized image storage
DIR_DOWNLOADED = "downloaded"      # Original images from Pokemon TCG API
DIR_CONVERTED = "converted"        # Processed PNG images with metadata
DIR_RAW = "raw"                   # Binary RGB565 files for ESP32 display

# Image Quality Settings
# These settings balance quality and performance
JPEG_QUALITY = 95                 # JPEG compression quality (0-100)
PNG_COMPRESSION = 6               # PNG compression level (0-9)
RESAMPLE_FILTER = "LANCZOS"       # High-quality resampling algorithm

# Performance Optimization
# Settings to optimize processing speed and memory usage
MAX_IMAGE_SIZE = 8192             # Maximum source image dimension (pixels)
CACHE_SIZE = 100                  # Maximum number of cached processed images
PARALLEL_WORKERS = 4              # Number of parallel processing workers

# Error Handling
# Configuration for robust error handling and retry logic
MAX_RETRY_ATTEMPTS = 3            # Maximum download retry attempts
RETRY_DELAY = 2                   # Base delay between retries (seconds)
TIMEOUT_SECONDS = 30              # HTTP request timeout (seconds)

# Debug Configuration
# Settings for development and troubleshooting
DEBUG_SAVE_INTERMEDIATE = False    # Save intermediate processing steps
DEBUG_VERBOSE_LOGGING = False      # Enable detailed logging output
DEBUG_COLOR_OVERLAY = False        # Show colored debug areas on images

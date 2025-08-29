#!/usr/bin/env python3
"""
Pokemon Card Expositor - Batch Processing Script

This script handles batch processing of multiple Pokemon cards from JSON
configuration files. It supports both direct image URLs and Pokemon TCG API
card IDs, making it versatile for different data sources and workflows.

Supported JSON Formats:
1. Pokemon TCG API format (with card IDs)
2. Direct image URLs with metadata
3. Mixed format with both approaches

Author: mrheltic
Date: August 2025
"""

import json
import os
import sys
import subprocess
import argparse
import time
from pathlib import Path
from typing import Dict, List, Any, Optional, Tuple
from concurrent.futures import ThreadPoolExecutor, as_completed

from pc_io import download_card_image, load_batch_file
from pc_convert import convert_single
from pc_utils import sanitize_filename, validate_card_id


def extract_card_metadata(card: Dict[str, Any]) -> Dict[str, Any]:
    """
    Extract and normalize card metadata from various JSON formats.
    
    This function handles metadata extraction from different JSON formats,
    including Pokemon TCG API responses and custom card databases.
    
    Args:
        card (Dict[str, Any]): Card data dictionary
        
    Returns:
        Dict[str, Any]: Normalized metadata dictionary
        
    Example:
        >>> card_data = {'name': 'Pikachu', 'set': {'name': 'Base Set'}, 'year': '1999'}
        >>> metadata = extract_card_metadata(card_data)
        >>> print(metadata['name'])  # 'Pikachu'
    """
    # Handle nested set information
    set_info = card.get('set', {})
    if isinstance(set_info, str):
        set_name = set_info
        release_date = card.get('year', '')
    else:
        set_name = set_info.get('name', '')
        release_date = set_info.get('releaseDate', card.get('year', ''))
    
    # Extract and normalize metadata fields
    metadata = {
        'id': card.get('id', ''),
        'name': card.get('name', ''),
        'set_name': set_name,
        'set_series': set_info.get('series', '') if isinstance(set_info, dict) else '',
        'release_date': release_date,
        'rarity': card.get('rarity', ''),
        'artist': card.get('artist', ''),
        'hp': card.get('hp', ''),
        'types': card.get('types', [])
    }
    
    # Clean up empty values
    return {k: v for k, v in metadata.items() if v}


def process_single_card(card: Dict[str, Any], output_dir: Path, 
                       force_overwrite: bool = False) -> Tuple[bool, str, str]:
    """
    Process a single card through the complete conversion pipeline.
    
    Args:
        card (Dict[str, Any]): Card information dictionary
        output_dir (Path): Output directory for processed files
        force_overwrite (bool): Whether to overwrite existing files
        
    Returns:
        Tuple[bool, str, str]: (success, card_id, error_message)
    """
    card_id = card.get('id', 'unknown')
    
    try:
        # Extract metadata
        metadata = extract_card_metadata(card)
        
        # Determine input source (card ID vs direct image URL)
        if validate_card_id(card_id):
            # Use Pokemon TCG API
            print(f"📥 Downloading card: {card_id}")
            input_path, api_metadata = download_card_image(card_id)
            
            # Merge API metadata with provided metadata
            metadata.update(api_metadata)
            
        elif 'image_url' in card:
            # Handle direct image URL
            image_url = card['image_url']
            downloaded_dir = Path('../images/downloaded')
            downloaded_dir.mkdir(exist_ok=True)
            
            # Generate filename from card info
            safe_name = sanitize_filename(card.get('name', card_id))
            image_filename = f"{card_id}_{safe_name}.png"
            input_path = downloaded_dir / image_filename
            
            # Download if not exists
            if not input_path.exists() or force_overwrite:
                import requests
                print(f"📥 Downloading from URL: {card_id}")
                response = requests.get(image_url, timeout=30)
                response.raise_for_status()
                
                with open(input_path, 'wb') as f:
                    f.write(response.content)
            
            input_path = str(input_path)
            
        else:
            return False, card_id, "No valid card ID or image URL provided"
        
        # Generate output paths
        safe_name = sanitize_filename(metadata.get('name', card_id))
        base_name = f"{card_id}_{safe_name}"
        
        output_png = output_dir / f"{base_name}_converted.png"
        output_raw = output_dir / f"{base_name}_1024x600.raw"
        
        # Check if files already exist
        if not force_overwrite and output_png.exists() and output_raw.exists():
            print(f"⏭️  Skipping {card_id}: files already exist")
            return True, card_id, ""
        
        # Convert the image
        print(f"🔄 Converting {card_id}: {metadata.get('name', 'Unknown')}")
        convert_single(input_path, str(output_png), str(output_raw), metadata)
        
        return True, card_id, ""
        
    except Exception as e:
        error_msg = f"Failed to process {card_id}: {str(e)}"
        print(f"❌ {error_msg}")
        return False, card_id, error_msg


def process_cards_batch(json_file: str, output_dir: str = 'converted', 
                       max_workers: int = 4, force_overwrite: bool = False,
                       resume_from: Optional[str] = None) -> Dict[str, Any]:
    """
    Process multiple cards from JSON configuration file with parallel processing.
    
    Args:
        json_file (str): Path to JSON configuration file
        output_dir (str): Output directory name within images/ folder
        max_workers (int): Maximum number of parallel processing threads
        force_overwrite (bool): Whether to overwrite existing files
        resume_from (Optional[str]): Card ID to resume processing from
        
    Returns:
        Dict[str, Any]: Processing results and statistics
    """
    # Load and validate JSON file
    try:
        with open(json_file, 'r', encoding='utf-8') as f:
            cards = json.load(f)
    except (FileNotFoundError, json.JSONDecodeError) as e:
        raise ValueError(f"Failed to load JSON file {json_file}: {e}")
    
    if not isinstance(cards, list):
        raise ValueError("JSON file must contain an array of card objects")
    
    # Setup output directory
    output_path = Path('../images') / output_dir
    output_path.mkdir(parents=True, exist_ok=True)
    
    # Handle resume functionality
    start_index = 0
    if resume_from:
        for i, card in enumerate(cards):
            if card.get('id') == resume_from:
                start_index = i
                print(f"🔄 Resuming from card {resume_from} (index {start_index})")
                break
        else:
            print(f"⚠️  Resume card {resume_from} not found, starting from beginning")
    
    # Initialize statistics
    stats = {
        'total': len(cards),
        'processed': 0,
        'successful': 0,
        'failed': 0,
        'skipped': 0,
        'errors': [],
        'start_time': time.time()
    }
    
    print(f"🚀 Starting batch processing: {stats['total']} cards")
    print(f"   📁 Output directory: {output_path}")
    print(f"   🔧 Max workers: {max_workers}")
    
    # Process cards with parallel execution
    cards_to_process = cards[start_index:]
    
    if max_workers == 1:
        # Sequential processing for debugging
        for i, card in enumerate(cards_to_process):
            success, card_id, error = process_single_card(card, output_path, force_overwrite)
            _update_stats(stats, success, card_id, error, start_index + i + 1)
    else:
        # Parallel processing for performance
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            # Submit all tasks
            future_to_card = {
                executor.submit(process_single_card, card, output_path, force_overwrite): (card, start_index + i)
                for i, card in enumerate(cards_to_process)
            }
            
            # Process completed tasks
            for future in as_completed(future_to_card):
                card, index = future_to_card[future]
                try:
                    success, card_id, error = future.result()
                    _update_stats(stats, success, card_id, error, index + 1)
                except Exception as e:
                    card_id = card.get('id', 'unknown')
                    _update_stats(stats, False, card_id, str(e), index + 1)
    
    # Calculate final statistics
    stats['end_time'] = time.time()
    stats['duration'] = stats['end_time'] - stats['start_time']
    
    # Print summary
    print(f"\n📊 Batch Processing Complete!")
    print(f"   ✅ Successful: {stats['successful']}")
    print(f"   ❌ Failed: {stats['failed']}")
    print(f"   ⏭️  Skipped: {stats['skipped']}")
    print(f"   ⏱️  Duration: {stats['duration']:.1f} seconds")
    
    if stats['errors']:
        print(f"\n❌ Errors encountered:")
        for error in stats['errors'][:10]:  # Show first 10 errors
            print(f"   • {error}")
        if len(stats['errors']) > 10:
            print(f"   ... and {len(stats['errors']) - 10} more errors")
    
    return stats


def _update_stats(stats: Dict[str, Any], success: bool, card_id: str, 
                 error: str, index: int) -> None:
    """Update processing statistics and display progress."""
    stats['processed'] += 1
    
    if success:
        if error == "":
            stats['successful'] += 1
            print(f"✅ [{index:3d}/{stats['total']}] {card_id}")
        else:
            stats['skipped'] += 1
            print(f"⏭️  [{index:3d}/{stats['total']}] {card_id} (skipped)")
    else:
        stats['failed'] += 1
        stats['errors'].append(f"{card_id}: {error}")
        print(f"❌ [{index:3d}/{stats['total']}] {card_id}")


def validate_json_format(json_file: str) -> bool:
    """
    Validate JSON file format and content.
    
    Args:
        json_file (str): Path to JSON file
        
    Returns:
        bool: True if format is valid, False otherwise
    """
    try:
        with open(json_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        if not isinstance(data, list):
            print("❌ JSON must contain an array of card objects")
            return False
        
        required_fields = ['id']
        optional_fields = ['name', 'image_url', 'set', 'rarity', 'artist']
        
        valid_count = 0
        for i, card in enumerate(data[:10]):  # Check first 10 cards
            if not isinstance(card, dict):
                print(f"❌ Card {i} is not an object")
                return False
            
            if not any(field in card for field in required_fields):
                print(f"❌ Card {i} missing required field: {required_fields}")
                return False
            
            if validate_card_id(card.get('id', '')):
                valid_count += 1
        
        print(f"✅ JSON format valid: {len(data)} cards, {valid_count} with valid IDs")
        return True
        
    except Exception as e:
        print(f"❌ JSON validation error: {e}")
        return False


def main():
    """Main function with command line interface."""
    parser = argparse.ArgumentParser(
        description='Batch process Pokemon cards from JSON configuration',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  # Basic batch processing
  %(prog)s cards.json
  
  # Custom output directory with parallel processing
  %(prog)s cards.json --output batch_1 --workers 8
  
  # Resume interrupted batch
  %(prog)s cards.json --resume sv10-150
  
  # Validate JSON format only
  %(prog)s cards.json --validate-only
        '''
    )
    
    parser.add_argument('json_file', help='JSON file containing card information')
    parser.add_argument('--output', '-o', default='converted',
                       help='Output directory name (default: converted)')
    parser.add_argument('--workers', '-w', type=int, default=4,
                       help='Number of parallel workers (default: 4)')
    parser.add_argument('--force', '-f', action='store_true',
                       help='Overwrite existing files')
    parser.add_argument('--resume', metavar='CARD_ID',
                       help='Resume processing from specific card ID')
    parser.add_argument('--validate-only', action='store_true',
                       help='Only validate JSON format, do not process')
    parser.add_argument('--sequential', action='store_true',
                       help='Use sequential processing (for debugging)')
    
    args = parser.parse_args()
    
    # Validate JSON file exists
    if not os.path.exists(args.json_file):
        print(f"❌ Error: JSON file not found: {args.json_file}")
        return 1
    
    # Validate JSON format
    if not validate_json_format(args.json_file):
        return 1
    
    if args.validate_only:
        print("✅ JSON validation completed successfully")
        return 0
    
    try:
        # Determine number of workers
        max_workers = 1 if args.sequential else args.workers
        
        # Process the batch
        stats = process_cards_batch(
            args.json_file,
            args.output,
            max_workers,
            args.force,
            args.resume
        )
        
        # Return appropriate exit code
        if stats['failed'] == 0:
            return 0
        elif stats['successful'] > 0:
            return 2  # Partial success
        else:
            return 3  # Complete failure
            
    except KeyboardInterrupt:
        print("\n⚠️  Batch processing interrupted by user")
        return 1
    except Exception as e:
        print(f"❌ Batch processing failed: {e}")
        return 1


if __name__ == '__main__':
    sys.exit(main())

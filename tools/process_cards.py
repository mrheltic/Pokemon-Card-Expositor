#!/usr/bin/env python3
"""
Process Pokemon cards from JSON file and convert them with metadata overlay.
"""
import json
import os
import sys
import subprocess
import requests
from pathlib import Path

def download_image(url, filepath):
    """Download image from URL to filepath."""
    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        with open(filepath, 'wb') as f:
            f.write(response.content)
        return True
    except Exception as e:
        print(f"Failed to download {url}: {e}")
        return False

def extract_card_metadata(card):
    """Extract relevant metadata from card JSON."""
    return {
        'name': card.get('name', ''),
        'set': card.get('set', ''),
        'year': card.get('year', ''),
        'rarity': card.get('rarity', ''),
        'artist': card.get('artist', '')
    }

def process_cards(json_file, output_dir='converted'):
    """Process all cards from JSON file."""
    # Load JSON
    with open(json_file, 'r') as f:
        cards = json.load(f)
    
    # Create output directories
    output_dir = Path('../images') / output_dir
    downloaded_dir = Path('../images/downloaded')
    output_dir.mkdir(exist_ok=True)
    downloaded_dir.mkdir(exist_ok=True)
    
    for card in cards:
        card_id = card.get('id', 'unknown')
        image_url = card.get('image', '')
        
        if not image_url:
            print(f"Skipping {card_id}: no image URL")
            continue
        
        # Download image if not exists
        image_filename = f"{card_id}.png"
        image_path = downloaded_dir / image_filename
        
        if not image_path.exists():
            print(f"Downloading {card_id}...")
            if not download_image(image_url, image_path):
                continue
        
        # Extract metadata
        metadata = extract_card_metadata(card)
        
        # Convert image
        output_png = output_dir / f"{card_id}_converted.png"
        output_raw = output_dir / f"{card_id}_converted.raw"
        
        print(f"Converting {card_id}: {card.get('name', 'Unknown')}")
        
        # Call pokemon_converter.py
        cmd = [
            sys.executable, 'pokemon_converter.py',
            str(image_path),
            str(output_png),
            str(output_raw),
            json.dumps(metadata)
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            if result.returncode == 0:
                print(f"  ✅ Converted {card_id}")
            else:
                print(f"  ❌ Failed {card_id}: {result.stderr}")
        except Exception as e:
            print(f"  ❌ Error {card_id}: {e}")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python3 process_cards.py <json_file> [output_dir]")
        sys.exit(1)
    
    json_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else 'converted'
    
    process_cards(json_file, output_dir)

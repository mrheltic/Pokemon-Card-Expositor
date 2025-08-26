#!/usr/bin/env python3
"""
Script per convertire un'immagine PNG in formato RAW RGB565 per ESP32
Usage: python3 convert_png_to_rgb565.py input.png output.raw [width] [height] [--rotate]
"""

import sys
from PIL import Image
import struct

def convert_png_to_rgb565(input_path, output_path, target_width=None, target_height=None, rotate_left=False):
    try:
        # Apri l'immagine
        img = Image.open(input_path)
        print(f"Immagine originale: {img.size[0]}x{img.size[1]}")
        
        # Ruota di 90 gradi a sinistra se richiesto
        if rotate_left:
            img = img.rotate(90, expand=True)
            print(f"Ruotata 90° a sinistra: {img.size[0]}x{img.size[1]}")
        
        # Ridimensiona se necessario
        if target_width and target_height:
            img = img.resize((target_width, target_height), Image.Resampling.LANCZOS)
            print(f"Ridimensionata a: {target_width}x{target_height}")
        
        # Converti in RGB se necessario
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Converti in RGB565
        rgb565_data = []
        width, height = img.size
        
        for y in range(height):
            for x in range(width):
                r, g, b = img.getpixel((x, y))
                
                # Converti a RGB565 (5 bit per R, 6 bit per G, 5 bit per B)
                r565 = (r >> 3) & 0x1F
                g565 = (g >> 2) & 0x3F  
                b565 = (b >> 3) & 0x1F
                
                # Combina in 16 bit
                rgb565 = (r565 << 11) | (g565 << 5) | b565
                rgb565_data.append(rgb565)
        
        # Scrivi nel file
        with open(output_path, 'wb') as f:
            # Scrivi header con dimensioni
            f.write(struct.pack('<HH', width, height))  # Little endian
            
            # Scrivi dati pixel
            for pixel in rgb565_data:
                f.write(struct.pack('<H', pixel))  # Little endian
        
        print(f"File convertito salvato in: {output_path}")
        print(f"Dimensioni: {width}x{height}")
        print(f"Dimensione file: {len(rgb565_data) * 2 + 4} bytes")
        
        return True
        
    except Exception as e:
        print(f"Errore durante la conversione: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 convert_png_to_rgb565.py input.png output.raw [width] [height] [--rotate]")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    # Controlla se c'è il flag di rotazione
    rotate_left = "--rotate" in sys.argv
    
    # Rimuovi il flag dall'elenco per non interferire con width/height
    args_without_rotate = [arg for arg in sys.argv if arg != "--rotate"]
    
    width = int(args_without_rotate[3]) if len(args_without_rotate) > 3 else None
    height = int(args_without_rotate[4]) if len(args_without_rotate) > 4 else None
    
    # Per il display 1024x600, se ruotiamo usiamo l'intero schermo
    if width is None and height is None:
        if rotate_left:
            # Immagine ruotata: usa l'intero schermo 1024x600
            width = 1024
            height = 600
        else:
            # Immagine normale: mantieni proporzioni
            width = 600
            height = 430
    
    convert_png_to_rgb565(input_file, output_file, width, height, rotate_left)

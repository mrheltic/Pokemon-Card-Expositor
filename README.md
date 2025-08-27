# Pokemon Expositor

Sistema ESP32-S3 per visualizzazione Pokemon cards con DMA ultra-veloce e architettura modulare.

## 📁 Struttura Progetto

```
PokemonExpositor/
├── 📟 Firmware/
│   ├── PokemonExpositor.ino      # Main program
│   ├── *_manager.cpp/h           # Modular managers  
│   ├── ESP_Panel_Conf.h          # LCD configuration
│   └── lv_conf.h                 # LVGL configuration
├── 🛠️ tools/
│   └── pokemon_converter.py      # Image conversion tool
├── 🖼️ sample_images/
│   └── *.raw                     # Sample converted images
└── 📖 README.md                  # This file
```

## 🔧 Hardware
- **Scheda**: Waveshare ESP32-S3 RGB LCD
- **Display**: LCD RGB 1024x600 pixel
- **Storage**: Scheda SD/microSD
- **Controllo**: PWM backlight + Serial interface

## ⚡ Funzionalità
- 🚀 **DMA Ultra-veloce**: 10-30x più veloce del normale
- 🔄 **Rotazione automatica**: 90° per display verticale ottimale
- 💾 **Formato ottimizzato**: RAW RGB565 per performance massime
- 🎮 **Interfaccia seriale**: Controllo completo via comandi
- 💡 **Controllo luminosità**: PWM con preset intelligenti
- 📦 **Architettura modulare**: Manager separati per ogni funzione

## 🎮 Comandi Disponibili

### Comandi Base
- `show [filename]` - Visualizza immagine RAW (o prima disponibile)
- `clear` - Pulisce schermo
- `list` - Lista immagini su SD card
- `status` - Stato sistema e memoria
- `help` - Lista completa comandi

### Controllo Luminosità
- `brightness night` - Notturna (5%)
- `brightness low` - Bassa (25%) 
- `brightness med` - Media (50%)
- `brightness high` - Alta (75%)
- `brightness max` - Massima (100%)
- `brightness <0-255>` - Valore personalizzato

## 🛠️ Conversione Immagini

### Script Unificato
```bash
cd tools/
python3 pokemon_converter.py <files...>
```

### Esempi d'Uso
```bash
# File singolo
python3 pokemon_converter.py card.png

# Multipli file  
python3 pokemon_converter.py *.jpg *.png

# Intera cartella
python3 pokemon_converter.py /path/to/images/

# Mix di opzioni
python3 pokemon_converter.py card1.png *.jpg folder/
```

### ✨ Caratteristiche Conversione
- 🔄 **Rotazione**: Automatica 90° antioraria per orientamento verticale
- � **Ridimensionamento**: Mantiene proporzioni + letterboxing nero 
- 🎯 **Risoluzione fissa**: Sempre 1024x600 pixel
- 💾 **Formato RAW**: RGB565 (1,228,800 bytes) per DMA ottimale
- 🚀 **Processamento multiplo**: File, pattern glob, cartelle

## Utilizzo
1. Formattare SD in FAT32
2. Convertire immagini con script Python
3. Copiare file RAW nella root della SD
4. Avviare sistema - l'immagine si carica automaticamente
5. Usare comandi seriali per controllo (115200 baud)

## Note Tecniche
- **DMA**: Transfer 10-30x più veloce del CPU standard
- **Formato**: Preferire RAW RGB565 per performance ottimali
- **Memory**: Gestione intelligente PSRAM/RAM interna
- **Fallback**: Sistema robusto con fallback CPU se DMA non disponibile
- **Risoluzione**: Fissa 1024x600 per performance ottimali

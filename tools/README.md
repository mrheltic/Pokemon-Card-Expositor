# Pokemon Converter Tool

Script Python unificato per la conversione di immagini al formato ottimale per Pokemon Expositor.

## 🚀 Requisiti

```bash
pip install Pillow
```

## 📖 Uso

```bash
python3 pokemon_converter.py <files...>
```

## 📋 Esempi

### File Singolo
```bash
python3 pokemon_converter.py my_pokemon.png
```

### Multipli File
```bash
python3 pokemon_converter.py *.jpg *.png *.bmp
```

### Cartella Completa
```bash
python3 pokemon_converter.py /path/to/pokemon_images/
```

### Mix di Opzioni
```bash
python3 pokemon_converter.py card1.png *.jpg /other/folder/
```

## ⚙️ Processo di Conversione

1. **Caricamento**: Legge file immagine (PNG, JPG, BMP, TIFF, WebP, GIF)
2. **Rotazione**: Ruota 90° antioraria per orientamento verticale
3. **Ridimensionamento**: Scale proporzionale per fit 1024x600
4. **Letterboxing**: Aggiunge barre nere per aspect ratio
5. **Conversione**: RGB888 → RGB565 per efficienza DMA
6. **Output**: File .raw pronto per SD card

## 📊 Output

- **Formato**: RAW RGB565 (2 bytes per pixel)
- **Risoluzione**: Sempre 1024x600 pixel
- **Dimensione**: Esattamente 1,228,800 bytes per file
- **Nome**: `original_name_1024x600.raw`

## 🎯 Ottimizzazioni

- **DMA Ready**: Formato ottimizzato per trasferimento DMA
- **Performance**: Conversione batch per molti file
- **Memoria**: Gestione efficiente per immagini grandi
- **Quality**: Resampling Lanczos per qualità massima

## 💡 Tips

- Usa immagini ad alta risoluzione per migliore qualità
- Il sistema gestisce automaticamente aspect ratio diversi
- File RAW vanno copiati nella root della SD card
- Preferisci formato PNG per immagini senza perdite

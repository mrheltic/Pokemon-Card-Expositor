# Sistema di Visualizzazione Immagini

## Panoramica
Il Pokemon Expositor ora supporta la visualizzazione di immagini dalla SD card con supporto per diversi formati.

## Formati Supportati

### 1. RAW RGB565 (Raccomandato)
- **Formato**: File binario con header + dati pixel
- **Estensioni**: `.raw`, `.rgb565`
- **Vantaggi**: Caricamento veloce, nessuna decodifica necessaria
- **Struttura**:
  ```
  Header: 4 bytes
  - Width (2 bytes, little-endian)
  - Height (2 bytes, little-endian)
  
  Dati Pixel: width * height * 2 bytes
  - Ogni pixel in formato RGB565 (2 bytes, little-endian)
  ```

### 2. PNG (Con Libreria)
- **Formato**: PNG standard
- **Estensioni**: `.png`
- **Vantaggi**: Compressione, qualità alta
- **Requisiti**: Libreria PNGdec installata
- **Stato**: Implementazione opzionale

### 3. Pattern di Test
- **Uso**: Quando nessun file immagine è disponibile
- **Funzione**: Mostra pattern colorati per test del display

## Conversione Immagini

### Da PNG a RAW RGB565
```bash
python3 convert_png_to_rgb565.py input.png output.raw [width] [height]
```

**Esempio**:
```bash
# Converte mantenendo proporzioni ottimali per il display 1024x600
python3 convert_png_to_rgb565.py 131_hires_1.png pokemon.raw

# Converte con dimensioni specifiche
python3 convert_png_to_rgb565.py input.png output.raw 800 480
```

### Requisiti per lo Script
```bash
pip3 install Pillow
```

## Configurazione

### project_config.h
```cpp
#define ENABLE_IMAGE_DISPLAY 1
#define DEFAULT_IMAGE_PATH "/pokemon.raw"
#define FALLBACK_IMAGE_PATH "/131_hires_1.png"

// Opzionale: per supporto PNG nativo
// #define USE_PNGDEC_LIBRARY 1
```

## Comandi Disponibili

### Comandi Seriali
```
image     - Esegui test completo del sistema immagini
show      - Mostra immagine di default
list      - Lista tutte le immagini sulla SD card
```

### Uso Programmatico
```cpp
// Visualizza immagine centrata
imageManager.displayPNGFromSD("/pokemon.raw");

// Visualizza a coordinate specifiche
imageManager.displayPNGFromSD("/pokemon.raw", 100, 50);

// Lista immagini disponibili
imageManager.listImagesOnSD();

// Controlla se un'immagine esiste
if (imageManager.checkImageExists("/pokemon.raw")) {
    // File trovato
}
```

## Dimensioni Ottimali

### Per Display 1024x600
- **Immagine piena**: 1024x600 pixel
- **Immagine centrata**: 600x430 pixel (mantiene proporzioni 4:3)
- **Immagine piccola**: 400x300 pixel

### Per Display 800x480
- **Immagine piena**: 800x480 pixel
- **Immagine centrata**: 480x360 pixel
- **Immagine piccola**: 320x240 pixel

## Performance

### Tempi di Caricamento (Stimati)
- **RAW RGB565 600x430**: ~2-3 secondi
- **PNG 600x430**: ~5-8 secondi (con decodifica)
- **Pattern Test**: ~1 secondo

### Memoria Utilizzata
- **Buffer riga**: ~12KB (per 600 pixel * 2 bytes * 10 righe)
- **Memoria totale**: ~20KB durante caricamento

## Workflow Consigliato

### 1. Preparazione Immagini
```bash
# Converti le tue immagini Pokemon
python3 convert_png_to_rgb565.py pokemon1.png pokemon1.raw
python3 convert_png_to_rgb565.py pokemon2.png pokemon2.raw
python3 convert_png_to_rgb565.py pokemon3.png pokemon3.raw
```

### 2. Caricamento su SD
- Copia i file `.raw` nella root della SD card
- Opzionalmente mantieni anche i PNG originali

### 3. Test del Sistema
```
> image    # Test completo
> list     # Verifica file presenti
> show     # Mostra immagine di default
```

## Risoluzione Problemi

### File Non Trovato
```
Image file not found: /pokemon.raw
```
**Soluzione**: Verifica che il file sia nella root della SD card

### Memoria Insufficiente
```
Failed to allocate line buffer
```
**Soluzione**: Riduci le dimensioni dell'immagine o il buffer size

### Display Corrotto
**Soluzioni**:
1. Verifica formato file (header corretto)
2. Controlla alimentazione del display
3. Riduci dimensioni immagine

### PNG Non Supportato
```
PNG library not available - showing test pattern
```
**Soluzione**: Usa file RAW RGB565 invece di PNG

## Esempi di File

### File Supportati sulla SD
```
/
├── pokemon.raw          # Immagine principale (600x430)
├── pokemon_full.raw     # Immagine full screen (1024x600)
├── pokemon_small.raw    # Immagine piccola (400x300)
├── 131_hires_1.png      # PNG originale (fallback)
└── test_pattern.raw     # Pattern di test personalizzato
```

### Dimensioni File
- **600x430 RAW**: ~516KB
- **1024x600 RAW**: ~1.2MB
- **400x300 RAW**: ~240KB

## Sviluppi Futuri

### Funzionalità Pianificate
- [ ] Slideshow automatico
- [ ] Transizioni tra immagini
- [ ] Supporto JPEG nativo
- [ ] Cache intelligente
- [ ] Zoom e pan
- [ ] Rotazione immagini
- [ ] Database delle carte Pokemon

### Ottimizzazioni
- [ ] Caricamento progressivo
- [ ] Compressione RAW personalizzata
- [ ] Double buffering
- [ ] DMA per trasferimenti veloci

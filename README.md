# Pokemon Expositor

Sistema ESP32-S3 per la visualizzazione di immagini Pokemon su display LCD con rotazione e supporto scheda SD.

## Hardware
- **Scheda**: Waveshare ESP32-S3 RGB LCD
- **Display**: LCD RGB 1024x600 pixel
- **Storage**: Scheda SD/microSD
- **Espansore IO**: CH422G (I2C)

## Funzionalità
- ✅ Visualizzazione immagini PNG da SD card
- ✅ Rotazione automatica 90° a sinistra per schermo intero
- ✅ Interfaccia seriale per controllo remoto
- ✅ Gestione modulare con manager dedicati
- ✅ Sistema di test integrato

## Struttura del Progetto

### File Principali
- `PokemonExpositor.ino` - Firmware principale
- `project_config.h` - Configurazione centrale
- `system_manager.h/cpp` - Coordinatore sistema
- `sd_manager.h/cpp` - Gestione SD card
- `lcd_manager.h/cpp` - Controllo display
- `image_manager.h/cpp` - Elaborazione immagini

### File di Supporto Hardware
- `waveshare_lcd_port.h/cpp` - Driver LCD Waveshare
- `waveshare_sd_card.h/cpp` - Driver SD Waveshare
- `sd_manager.c` - Implementazione SD legacy
- `test_functions.h` - Funzioni di test

## Comandi Seriali

### Visualizzazione Immagini
- `show` - Mostra Pokemon ruotato a schermo intero
- `show-normal` - Mostra Pokemon centrato normale
- `clear` - Pulisci schermo

### Sistema
- `status` - Stato del sistema
- `list` - Lista immagini su SD
- `restart` - Riavvia sistema
- `help` - Mostra tutti i comandi

### Test
- `test` - Esegui tutti i test
- `sd` - Test SD card
- `lcd` - Test LCD
- `image` - Test sistema immagini

## Utilizzo

1. **Preparazione SD Card**:
   - Formattare in FAT32
   - Copiare immagini PNG nella root

2. **Avvio Sistema**:
   - Collegare alimentazione
   - Aprire monitor seriale (115200 baud)
   - Il sistema si avvia automaticamente

3. **Controllo**:
   - Usare comandi seriali per controllo
   - L'immagine si carica automaticamente all'avvio

## Note Tecniche

- **Formato Immagini**: Supporta PNG con conversione automatica RGB565
- **Rotazione**: 90° sinistra per ottimizzare spazio schermo
- **Buffer**: Gestione dinamica memoria per immagini grandi
- **Performance**: Ottimizzato per display senza flickering

## Troubleshooting

- **Bordo bianco**: Comando `calibrate` per correggere
- **Glitch superiori**: Comando `clear-top` per pulire
- **SD non rilevata**: Verificare connessioni e formato FAT32
- **Immagine non carica**: Verificare nome file e formato PNG

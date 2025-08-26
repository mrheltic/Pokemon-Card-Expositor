# Pokemon Expositor

Sistema ESP32-S3 per visualizzazione Pokemon cards con DMA ultra-veloce.

## Hardware
- **Scheda**: Waveshare ESP32-S3 RGB LCD
- **Display**: LCD RGB 1024x600 pixel
- **Storage**: Scheda SD/microSD

## Funzionalità
- ⚡ Visualizzazione immagini ultra-veloce con DMA
- 🔄 Rotazione automatica 90° per schermo intero
- 💾 Supporto PNG e RAW RGB565
- 🎮 Interfaccia seriale per controllo

## Comandi Base
- `show` - Visualizza Pokemon rotato a schermo intero
- `clear` - Pulisce schermo
- `list` - Lista immagini su SD
- `status` - Stato sistema
- `help` - Tutti i comandi

## Utilizzo
1. Formattare SD in FAT32
2. Copiare immagini PNG nella root
3. Avviare sistema - l'immagine si carica automaticamente
4. Usare comandi seriali per controllo (115200 baud)

## Note Tecniche
- **DMA**: Transfer 10-30x più veloce del CPU standard
- **Formato**: Supporta PNG con conversione automatica RGB565
- **Memory**: Gestione intelligente PSRAM/RAM interna
- **Fallback**: Sistema robusto con fallback CPU se DMA non disponibile

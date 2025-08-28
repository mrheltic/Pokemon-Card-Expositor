# Pokemon Card Exporter & Converter

Una pipeline completa per cercare, salvare, esportare e convertire carte Pokémon per Pokemon Expositor.

## 🚀 Requisiti

### Node.js (per il server web)
```bash
npm install
```

### Python (per la conversione immagini)
```bash
pip install Pillow
```

## � Come usare

### 1. Avvia il server
```bash
npm start
```
Apri `http://localhost:3000` nel browser

### 2. Cerca e salva carte
- Usa i filtri per cercare carte specifiche
- Clicca sulle carte per salvarle nella lista
- "Add All to List" salva tutti i risultati

### 3. Esporta e converti
- Clicca "Export Cards" per elaborare le carte
- La pipeline scarica e converte automaticamente le immagini

## 🔄 Pipeline automatica

1. **Ricerca**: API ufficiale Pokémon TCG
2. **Salvataggio**: Lista locale nel browser
3. **Esportazione**: Download immagini HD
4. **Conversione**: RGB565 RAW per ESP32
5. **Output**: File pronti per SD card

## 📁 Struttura output

```
sample_images/exported/
├── exported_cards_TIMESTAMP.json  # Metadati carte
├── images/                        # PNG originali
└── raw/                          # File RAW 1024x600
```

## ⚙️ Processo di conversione

1. **Caricamento**: Legge file immagine (PNG, JPG, BMP, TIFF, WebP, GIF)
2. **Rotazione**: Ruota 90° antioraria per orientamento verticale
3. **Ridimensionamento**: Scale proporzionale per fit 1024x600
4. **Letterboxing**: Aggiunge barre nere per aspect ratio
5. **Conversione**: RGB888 → RGB565 per efficienza DMA
6. **Output**: File .raw pronto per SD card

## 📊 Specifiche tecniche

- **Risoluzione**: 1024x600 pixel
- **Formato**: RAW RGB565 (2 bytes per pixel)
- **Orientamento**: Verticale (rotazione 90°)
- **Filesystem**: Compatibile con SD card ESP32

## 🔧 API Endpoints

- `GET /` - Interfaccia web
- `GET /api/search?q=query` - Ricerca carte
- `POST /api/export` - Esporta e converte

## 🐛 Troubleshooting

- **Errore rete**: Verifica connessione per API Pokémon
- **Python non trovato**: `python3` deve essere nel PATH
- **Permessi**: Server deve poter scrivere in `sample_images/exported/`

## 📋 Uso manuale del convertitore

```bash
python3 pokemon_converter.py <files...>
```

Esempi:
```bash
python3 pokemon_converter.py image.png
python3 pokemon_converter.py *.jpg
python3 pokemon_converter.py /path/to/images/
```
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

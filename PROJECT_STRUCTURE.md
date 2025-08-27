# 📁 Struttura Progetto Pokemon Expositor

```
PokemonExpositor/
├── 📟 FIRMWARE CORE
│   ├── PokemonExpositor.ino           # Main program entry point
│   ├── project_config.h               # Global configuration
│   ├── ESP_Panel_Conf.h               # LCD panel configuration
│   └── lv_conf.h                      # LVGL library configuration
│
├── 🔧 SYSTEM MANAGERS (Architettura Modulare)
│   ├── system_manager.cpp/h           # Central system coordinator
│   ├── dma_image_manager.cpp/h        # Ultra-fast DMA image display
│   ├── brightness_manager.cpp/h       # PWM backlight control
│   ├── lcd_manager.cpp/h              # LCD operations
│   ├── sd_manager.cpp/h               # SD card management
│   └── image_browser.cpp/h            # Image navigation system
│
├── 🖥️ HARDWARE DRIVERS
│   ├── waveshare_lcd_port.cpp/h       # Waveshare LCD driver
│   └── waveshare_sd_card.cpp/h        # SD card driver
│
├── 🛠️ tools/                          # DEVELOPMENT TOOLS
│   ├── pokemon_converter.py           # Universal image converter
│   └── README.md                      # Tool documentation
│
├── 🖼️ sample_images/                  # EXAMPLE FILES
│   ├── *.raw                          # Sample converted images
│   └── (your converted images here)
│
├── 📖 README.md                       # Main documentation
└── .vscode/                          # VS Code settings
```

## 🎯 **Benefici Struttura Modulare**

### 🚀 **Performance**
- **DMA Ultra-veloce**: 10-30x più veloce del CPU standard
- **Gestione memoria**: PSRAM/RAM interna intelligente
- **Format ottimizzato**: RGB565 RAW per performance massime

### 🔧 **Manutenibilità** 
- **Manager separati**: Ogni funzione ha il suo modulo
- **Dipendenze chiare**: Accoppiamento ridotto
- **Debug facilitato**: Isolamento problemi per modulo

### 📦 **Organizzazione**
- **Firmware**: File core nella root per compilazione
- **Tools**: Script di sviluppo separati
- **Assets**: Immagini di esempio organizzate
- **Docs**: Documentazione unificata

## 🔄 **Workflow Sviluppo**

1. **Sviluppo Firmware**: Modifica manager specifici
2. **Conversione Assets**: Usa `tools/pokemon_converter.py`
3. **Test**: Usa immagini in `sample_images/`
4. **Deploy**: Copia RAW su SD card

## 🎮 **Ready to Use!**

La codebase è ora ottimizzata, pulita e pronta per l'uso professionale! 🌟

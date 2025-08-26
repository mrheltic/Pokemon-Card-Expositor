#include "image_manager.h"
#include "project_config.h"

// Include per PNG decoder - useremo PNGdec library
// Se non disponibile, implementeremo un decoder semplice
#ifdef USE_PNGDEC_LIBRARY
#include <PNGdec.h>
#endif

ImageManager imageManager;

// Callback per PNGdec
#ifdef USE_PNGDEC_LIBRARY
PNG png;
File pngFile;

void * myOpen(const char *filename, int32_t *size) {
    pngFile = SD.open(filename);
    if (pngFile) {
        *size = pngFile.size();
        return &pngFile;
    }
    return nullptr;
}

void myClose(void *handle) {
    File *f = static_cast<File*>(handle);
    if (f) f->close();
}

int32_t myRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
    File *f = static_cast<File*>(page->fHandle);
    return f->read(buffer, length);
}

int32_t mySeek(PNGFILE *page, int32_t position) {
    File *f = static_cast<File*>(page->fHandle);
    return f->seek(position);
}

void PNGDraw(PNGDRAW *pDraw) {
    // Questa funzione viene chiamata per ogni riga decodificata
    // Dobbiamo convertire e inviare al display
    if (imageManager.isInitialized()) {
        auto lcd = waveshare_lcd_get_instance();
        if (lcd) {
            // Converte la riga in RGB565 e la disegna
            uint16_t* lineBuffer = (uint16_t*)malloc(pDraw->iWidth * 2);
            if (lineBuffer) {
                for (int x = 0; x < pDraw->iWidth; x++) {
                    uint8_t r, g, b;
                    if (pDraw->ucPixelType == PNG_PIXEL_TRUECOLOR) {
                        r = pDraw->pPixels[x * 3];
                        g = pDraw->pPixels[x * 3 + 1];
                        b = pDraw->pPixels[x * 3 + 2];
                    } else {
                        r = g = b = pDraw->pPixels[x]; // Grayscale
                    }
                    // Converte a RGB565
                    lineBuffer[x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }
                
                // Disegna la riga sul display
                lcd->drawBitmap(pDraw->x, pDraw->y + pDraw->iRow, pDraw->iWidth, 1, (uint8_t*)lineBuffer);
                free(lineBuffer);
            }
        }
    }
}
#endif

ImageManager::ImageManager() : initialized(false) {
}

ImageManager::~ImageManager() {
    deinit();
}

bool ImageManager::init() {
    if (initialized) {
        return true;
    }

    Serial.println("Initializing Image Manager...");
    
    // Verifica che SD e LCD siano inizializzati
    if (!sdManager.isInitialized()) {
        Serial.println("SD Manager must be initialized first");
        return false;
    }
    
    if (!lcdManager.isInitialized()) {
        Serial.println("LCD Manager must be initialized first");
        return false;
    }

#ifdef USE_PNGDEC_LIBRARY
    // Configura PNGdec
    png.open(myOpen, myClose, myRead, mySeek, PNGDraw);
#endif
    
    initialized = true;
    Serial.println("Image Manager initialized successfully");
    return true;
}

void ImageManager::deinit() {
    if (initialized) {
        initialized = false;
        Serial.println("Image Manager deinitialized");
    }
}

bool ImageManager::checkImageExists(const char* filepath) {
    if (!sdManager.isInitialized()) {
        Serial.println("SD Manager not initialized");
        return false;
    }
    
    File file = SD.open(filepath);
    if (file) {
        file.close();
        return true;
    }
    return false;
}

void ImageManager::listImagesOnSD() {
    if (!sdManager.isInitialized()) {
        Serial.println("SD Manager not initialized");
        return;
    }
    
    Serial.println("Searching for images on SD card...");
    File root = SD.open("/");
    File file = root.openNextFile();
    
    while (file) {
        String filename = file.name();
        filename.toLowerCase();
        
        if (filename.endsWith(".png") || filename.endsWith(".jpg") || 
            filename.endsWith(".jpeg") || filename.endsWith(".bmp") ||
            filename.endsWith(".raw") || filename.endsWith(".rgb565")) {
            Serial.printf("Found image: %s (size: %d bytes)\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
    root.close();
}

bool ImageManager::displayPNGFromSD(const char* filepath) {
    return displayPNGFromSD(filepath, 0, 0);
}

bool ImageManager::displayPNGFromSD(const char* filepath, uint16_t x, uint16_t y) {
    return displayPNGFromSD(filepath, x, y, EXAMPLE_LCD_WIDTH, EXAMPLE_LCD_HEIGHT);
}

bool ImageManager::displayPNGFromSD(const char* filepath, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {
    if (!initialized) {
        Serial.println("Image Manager not initialized");
        return false;
    }
    
    if (!checkImageExists(filepath)) {
        Serial.printf("Image file not found: %s\n", filepath);
        return false;
    }
    
    Serial.printf("Loading and displaying image: %s\n", filepath);
    
    // Prima prova a vedere se è un file RAW RGB565 (più semplice)
    String filename = String(filepath);
    if (filename.endsWith(".raw") || filename.endsWith(".rgb565")) {
        return displayRAWRGB565(filepath, x, y);
    }
    
#ifdef USE_PNGDEC_LIBRARY
    // Usa PNGdec library se disponibile
    if (png.openSD((char*)filepath) == PNG_SUCCESS) {
        Serial.printf("Image specs: (%dx%d), %d bpp, pixel type: %d\n", 
                     png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
        
        int result = png.decode(nullptr, 0);
        png.close();
        
        if (result == PNG_SUCCESS) {
            Serial.println("Image displayed successfully");
            return true;
        } else {
            Serial.printf("PNG decode failed: %d\n", result);
            return false;
        }
    } else {
        Serial.println("Failed to open PNG file");
        return false;
    }
#else
    // Per file PNG senza libreria, mostra pattern di test
    Serial.println("PNG library not available - showing test pattern");
    Serial.println("Convert your PNG to RAW RGB565 format for direct loading");
    displayTestPattern();
    return true;
#endif
}

bool ImageManager::displayRAWRGB565(const char* filepath, uint16_t x, uint16_t y) {
    File file = SD.open(filepath);
    if (!file) {
        Serial.printf("Cannot open RAW file: %s\n", filepath);
        return false;
    }
    
    // Leggi header (width, height)
    uint16_t width, height;
    if (file.read((uint8_t*)&width, 2) != 2 || file.read((uint8_t*)&height, 2) != 2) {
        Serial.println("Failed to read RAW file header");
        file.close();
        return false;
    }
    
    Serial.printf("RAW RGB565 image: %dx%d\n", width, height);
    
    auto lcd = waveshare_lcd_get_instance();
    if (!lcd) {
        Serial.println("LCD instance not available");
        file.close();
        return false;
    }
    
    // Pulisce solo la parte superiore dello schermo per eliminare glitch
    Serial.println("Clearing top area to fix potential glitches...");
    uint8_t* clearBuffer = (uint8_t*)malloc(EXAMPLE_LCD_WIDTH * 2 * 20); // 20 righe superiori
    if (clearBuffer) {
        memset(clearBuffer, 0, EXAMPLE_LCD_WIDTH * 2 * 20); // Riempie con nero
        lcd->drawBitmap(0, 0, EXAMPLE_LCD_WIDTH, 20, clearBuffer);
        free(clearBuffer);
    }
    
    // Calcola posizione centrata se x,y sono 0 E l'immagine non è a schermo intero
    if (x == 0 && y == 0) {
        if (width == EXAMPLE_LCD_WIDTH && height == EXAMPLE_LCD_HEIGHT) {
            // Immagine a schermo intero - usa posizione 0,0
            x = 0;
            y = 0;
            Serial.println("Full screen image detected - no centering");
        } else {
            // Centra l'immagine
            x = (EXAMPLE_LCD_WIDTH - width) / 2;
            y = (EXAMPLE_LCD_HEIGHT - height) / 2;
        }
    }
    
    Serial.printf("Displaying at position (%d, %d)\n", x, y);
    
    // Riduce il numero di righe per buffer per evitare glitch
    const uint16_t bufferLines = 5; // Ridotto da 10 a 5 per stabilità
    uint8_t* lineBuffer = (uint8_t*)malloc(width * 2 * bufferLines);
    if (!lineBuffer) {
        Serial.println("Failed to allocate line buffer");
        file.close();
        return false;
    }
    
    Serial.println("Starting image transfer - optimized for glitch reduction");
    
    for (uint16_t row = 0; row < height; row += bufferLines) {
        uint16_t linesToRead = min((uint16_t)bufferLines, (uint16_t)(height - row));
        size_t bytesToRead = width * 2 * linesToRead;
        
        if (file.read(lineBuffer, bytesToRead) != bytesToRead) {
            Serial.printf("Failed to read image data at row %d\n", row);
            break;
        }
        
        // Disegna le righe lette
        lcd->drawBitmap(x, y + row, width, linesToRead, lineBuffer);
        
        // Piccola pausa per stabilizzare il trasferimento (opzionale)
        if (row == 0) {
            Serial.println("First rows transferred - checking for glitches...");
        }
    }
    
    free(lineBuffer);
    file.close();
    
    Serial.println("RAW RGB565 image displayed successfully");
    return true;
}

// Implementazione semplice per file BMP (più facile da decodificare)
bool ImageManager::displaySimpleBMP(const char* filepath, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {
    File file = SD.open(filepath);
    if (!file) {
        Serial.printf("Cannot open file: %s\n", filepath);
        return false;
    }
    
    // Per ora, creiamo una implementazione di test che mostra un pattern
    // In una implementazione completa, qui decodificheremmo il BMP
    Serial.println("Displaying test pattern instead of actual image");
    displayTestPattern();
    
    file.close();
    return true;
}

void ImageManager::displayTestPattern() {
    if (!initialized) {
        Serial.println("Image Manager not initialized");
        return;
    }
    
    auto lcd = waveshare_lcd_get_instance();
    if (!lcd) {
        Serial.println("LCD instance not available");
        return;
    }
    
    Serial.println("Drawing test image pattern...");
    
    // Crea un pattern di test che simula un'immagine
    const uint16_t width = 200;
    const uint16_t height = 150;
    const uint16_t startX = (EXAMPLE_LCD_WIDTH - width) / 2;
    const uint16_t startY = (EXAMPLE_LCD_HEIGHT - height) / 2;
    
    // Alloca buffer per una riga
    uint8_t* lineBuffer = (uint8_t*)malloc(width * 2); // RGB565 = 2 bytes per pixel
    if (!lineBuffer) {
        Serial.println("Failed to allocate line buffer");
        return;
    }
    
    // Disegna pattern a righe colorate
    for (uint16_t y = 0; y < height; y++) {
        uint16_t* pixels = (uint16_t*)lineBuffer;
        
        for (uint16_t x = 0; x < width; x++) {
            uint16_t color;
            
            // Crea un pattern colorato
            if (y < height / 3) {
                color = 0xF800; // Rosso
            } else if (y < (height * 2) / 3) {
                color = 0x07E0; // Verde
            } else {
                color = 0x001F; // Blu
            }
            
            // Aggiungi variazione orizzontale
            if (x % 20 < 10) {
                color = (color >> 1) & 0x7BEF; // Dimezza l'intensità
            }
            
            pixels[x] = color;
        }
        
        // Disegna la riga
        lcd->drawBitmap(startX, startY + y, width, 1, lineBuffer);
    }
    
    free(lineBuffer);
    Serial.println("Test pattern displayed");
}

void ImageManager::runImageTest() {
    if (!initialized) {
        Serial.println("Image Manager not initialized - attempting to initialize...");
        if (!init()) {
            Serial.println("Failed to initialize Image Manager");
            return;
        }
    }
    
    Serial.println("=== Image Test Started ===");
    
    // Lista le immagini sulla SD
    listImagesOnSD();
    
    // Prova a caricare l'immagine ruotata a schermo intero se disponibile
    const char* testImageRotated = "/pokemon_rotated.raw";
    Serial.printf("Attempting to display rotated full-screen image: %s\n", testImageRotated);
    
    if (checkImageExists(testImageRotated)) {
        if (displayPNGFromSD(testImageRotated)) {
            Serial.println("Rotated full-screen image displayed successfully");
        } else {
            Serial.println("Rotated image display failed, trying fallback...");
            tryFallbackImages();
        }
    } else {
        Serial.println("Rotated image not found, trying fallback images...");
        tryFallbackImages();
    }
    
    Serial.println("=== Image Test Completed ===");
}

void ImageManager::tryFallbackImages() {
    // Prova con l'immagine RAW normale
    const char* testImageRAW = FALLBACK_IMAGE_PATH;
    Serial.printf("Attempting to display: %s\n", testImageRAW);
    
    if (checkImageExists(testImageRAW)) {
        if (displayPNGFromSD(testImageRAW)) {
            Serial.println("Fallback RAW image displayed successfully");
            return;
        }
    }
    
    // Prova con il PNG originale
    const char* testImagePNG = ORIGINAL_PNG_PATH;
    Serial.printf("Attempting to display PNG image: %s\n", testImagePNG);
    
    if (checkImageExists(testImagePNG)) {
        if (!displayPNGFromSD(testImagePNG)) {
            Serial.println("PNG display failed, showing test pattern instead");
            displayTestPattern();
        }
    } else {
        Serial.println("No images found, showing test pattern");
        displayTestPattern();
    }
}

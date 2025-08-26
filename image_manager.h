#ifndef __IMAGE_MANAGER_H
#define __IMAGE_MANAGER_H

#include <Arduino.h>
#include "sd_manager.h"
#include "lcd_manager.h"

// PNG decoder library - we'll need to include a PNG library
// For ESP32, we can use PNGdec library or implement a simple decoder

class ImageManager {
private:
    bool initialized;
    
    // Image processing functions
    bool loadPNGFromSD(const char* filepath, uint8_t** imageData, uint16_t* width, uint16_t* height);
    bool convertToRGB565(uint8_t* pngData, uint8_t** rgb565Data, uint16_t width, uint16_t height, size_t pngDataSize);
    void scaleImage(uint8_t* srcData, uint16_t srcWidth, uint16_t srcHeight, 
                   uint8_t** dstData, uint16_t dstWidth, uint16_t dstHeight);
    bool displaySimpleBMP(const char* filepath, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight);
    bool displayRAWRGB565(const char* filepath, uint16_t x, uint16_t y);
    void tryFallbackImages();

public:
    ImageManager();
    ~ImageManager();
    
    bool init();
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Main functions
    bool displayPNGFromSD(const char* filepath);
    bool displayPNGFromSD(const char* filepath, uint16_t x, uint16_t y);
    bool displayPNGFromSD(const char* filepath, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight);
    bool displayWithRotation(const char* filepath, int rotation = 0); // 0=none, 90=left, 180=flip, 270=right
    
    // Utility functions
    bool checkImageExists(const char* filepath);
    void listImagesOnSD();
    
    // Test functions
    void runImageTest();
    void displayTestPattern();
};

extern ImageManager imageManager;

#endif

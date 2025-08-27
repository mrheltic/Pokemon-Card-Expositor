/**
 * DMA-Enhanced Image Manager for Pokemon Expositor
 * 
 * Utilizza ESP32-S3 DMA per trasferimenti ultra-rapidi
 * Ottimizzato per immagini ESATTE 1024x600 RGB565
 * IMPORTANTE: Tutte le immagini devono essere preparate a 1024x600!
 */

#ifndef DMA_IMAGE_MANAGER_H
#define DMA_IMAGE_MANAGER_H

#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "SD.h"
#include "project_config.h"

// Fixed image dimensions - ALL images must be exactly this size!
#define FIXED_IMAGE_WIDTH   1024
#define FIXED_IMAGE_HEIGHT  600
#define FIXED_IMAGE_SIZE    (FIXED_IMAGE_WIDTH * FIXED_IMAGE_HEIGHT * 2)  // RGB565 = 2 bytes per pixel
#define FIXED_PIXELS_TOTAL  (FIXED_IMAGE_WIDTH * FIXED_IMAGE_HEIGHT)

class DMAImageManager {
private:
    // DMA Configuration
    uint8_t* dmaBuffer;
    size_t dmaBufferSize;
    bool dmaEnabled;
    bool initialized;
    size_t maxChunkSize;
    
public:
    DMAImageManager();
    ~DMAImageManager();
    
    // Basic image manager functionality
    bool init();
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Enhanced initialization with DMA setup
    bool initWithDMA();
    
    // DMA-accelerated image display for FIXED 1024x600 images
    bool displayImageDMA(const char* filepath);
    bool displayFixedSizeImageDMA(const char* filepath);  // NEW: Optimized for exact 1024x600
    bool displayRAWRGB565DMA(const char* filepath);
    bool displayPNGFromSD(const char* filepath);
    
    // DMA management
    bool enableDMA();
    void disableDMA();
    bool isDMAEnabled() const { return dmaEnabled; }
    
    // Testing
    void runImageTest();
    
private:
    // DMA setup functions
    bool allocateDMABuffer();
    void freeDMAResources();
    
    // Optimized transfer functions
    bool transferChunkOptimized(uint8_t* imageData, size_t size, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    bool memcpyDMA(void* dst, const void* src, size_t size);
};

extern DMAImageManager dmaImageManager;

#endif // DMA_IMAGE_MANAGER_H

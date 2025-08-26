/**
 * DMA-Enhanced Image Manager for Pokemon Expositor
 * 
 * Utilizza ESP32-S3 DMA per trasferimenti ultra-rapidi
 * Ottimizzato per immagini statiche grandi (Pokemon cards)
 */

#ifndef DMA_IMAGE_MANAGER_H
#define DMA_IMAGE_MANAGER_H

#include "image_manager.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

class DMAImageManager : public ImageManager {
private:
    // DMA Configuration
    uint8_t* dmaBuffer;
    size_t dmaBufferSize;
    bool dmaEnabled;
    size_t maxChunkSize;
    
public:
    DMAImageManager();
    ~DMAImageManager();
    
    // Enhanced initialization with DMA setup
    bool initWithDMA();
    
    // DMA-accelerated image display
    bool displayImageDMA(const char* filepath);
    bool displayRAWRGB565DMA(const char* filepath);
    
    // DMA management
    bool enableDMA();
    void disableDMA();
    bool isDMAEnabled() const { return dmaEnabled; }
    
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

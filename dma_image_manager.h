/**
 * DMA-Enhanced Image Manager for Pokemon Expositor
 * 
 * Utilizza ESP32-S3 DMA per trasferimenti ultra-rapidi
 * Ottimizzato per immagini statiche grandi (Pokemon cards)
 */

#ifndef DMA_IMAGE_MANAGER_H
#define DMA_IMAGE_MANAGER_H

#include "image_manager.h"
#include "driver/dma.h"
#include "esp_dma_utils.h"

class DMAImageManager : public ImageManager {
private:
    // DMA Configuration
    dma_descriptor_t* dmaDescriptors;
    uint8_t* dmaBuffer;
    size_t dmaBufferSize;
    bool dmaEnabled;
    
    // Performance tracking
    unsigned long lastTransferTime;
    size_t bytesPerSecond;
    
public:
    DMAImageManager();
    ~DMAImageManager();
    
    // Enhanced initialization with DMA setup
    bool initWithDMA();
    
    // DMA-accelerated image display
    bool displayImageDMA(const char* filepath);
    bool displayRAWRGB565DMA(const char* filepath);
    
    // Performance monitoring
    void printDMAPerformance();
    
    // DMA management
    bool enableDMA();
    void disableDMA();
    bool isDMAEnabled() const { return dmaEnabled; }
    
private:
    // DMA setup functions
    bool setupDMADescriptors();
    bool allocateDMABuffer();
    void freeDMAResources();
    
    // DMA transfer functions
    bool transferToDMABuffer(uint8_t* imageData, size_t size);
    bool startDMATransfer();
    void waitForDMAComplete();
    
    // Performance helpers
    void startPerformanceTimer();
    void endPerformanceTimer(size_t bytesTransferred);
};

extern DMAImageManager dmaImageManager;

#endif // DMA_IMAGE_MANAGER_H

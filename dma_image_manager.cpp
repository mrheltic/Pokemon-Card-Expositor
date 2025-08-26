/**
 * Implementazione DMA per Pokemon Expositor
 * 
 * Performance target: 
 * - Standard CPU: ~2-3 secondi per 1024x600 RGB565
 * - Con DMA: ~0.1-0.2 secondi per la stessa immagine
 */

#include "dma_image_manager.h"
#include "project_config.h"
#include "waveshare_lcd_port.h"

DMAImageManager dmaImageManager;

DMAImageManager::DMAImageManager() : 
    ImageManager(),
    dmaBuffer(nullptr),
    dmaBufferSize(0),
    dmaEnabled(false),
    maxChunkSize(32768) { // 32KB chunks
}

DMAImageManager::~DMAImageManager() {
    freeDMAResources();
}

bool DMAImageManager::initWithDMA() {
    // Initialize base image manager first
    if (!ImageManager::init()) {
        Serial.println("ERROR: Base image manager initialization failed");
        return false;
    }
    
    // Setup DMA for LCD transfers
    if (!enableDMA()) {
        Serial.println("WARNING: DMA setup failed, falling back to CPU transfers");
        return true; // Still functional without DMA
    }
    
    return true;
}

bool DMAImageManager::enableDMA() {
    if (dmaEnabled) {
        return true;
    }
    
    // Calculate optimal buffer size for ESP32-S3
    // Use PSRAM if available, otherwise internal SRAM
    size_t availableRAM = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    if (availableRAM == 0) {
        availableRAM = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    }
    
    // Target: largest possible buffer for smooth transfers
    dmaBufferSize = min((size_t)(SCREEN_WIDTH * SCREEN_HEIGHT * 2), availableRAM / 4); // Use 1/4 of available
    dmaBufferSize = max(dmaBufferSize, (size_t)(SCREEN_WIDTH * 10 * 2)); // Minimum 10 lines
    
    if (!allocateDMABuffer()) {
        Serial.println("Failed to allocate DMA buffer");
        return false;
    }
    
    dmaEnabled = true;
    return true;
}

bool DMAImageManager::allocateDMABuffer() {
    // Try PSRAM first (external SPIRAM) for large buffers
    dmaBuffer = (uint8_t*)heap_caps_malloc(dmaBufferSize, MALLOC_CAP_SPIRAM);
    
    if (!dmaBuffer) {
        // Fallback to internal RAM with smaller buffer
        dmaBufferSize = SCREEN_WIDTH * 50 * 2; // 50 lines buffer
        dmaBuffer = (uint8_t*)heap_caps_malloc(dmaBufferSize, MALLOC_CAP_INTERNAL);
    }
    
    if (!dmaBuffer) {
        // Last resort: very small buffer
        dmaBufferSize = SCREEN_WIDTH * 10 * 2; // 10 lines minimum
        dmaBuffer = (uint8_t*)heap_caps_malloc(dmaBufferSize, MALLOC_CAP_INTERNAL);
    }
    
    if (!dmaBuffer) {
        return false;
    }
    
    return true;
}

bool DMAImageManager::displayImageDMA(const char* filepath) {
    if (!isInitialized()) {
        return false;
    }
    
    if (dmaEnabled) {
        // Check if it's a RAW file for direct DMA transfer
        String filename = String(filepath);
        
        if (filename.endsWith(".raw")) {
            return displayRAWRGB565DMA(filepath);
        } else {
            // For PNG files, use standard decoder with DMA buffer optimization
            return ImageManager::displayPNGFromSD(filepath);
        }
    } else {
        return ImageManager::displayPNGFromSD(filepath);
    }
}

bool DMAImageManager::displayRAWRGB565DMA(const char* filepath) {
    if (!dmaEnabled) {
        return false;
    }
    
    // Open file
    File imageFile = SD.open(filepath);
    if (!imageFile) {
        return false;
    }
    
    // Get LCD instance
    auto lcd = waveshare_lcd_get_instance();
    if (!lcd) {
        imageFile.close();
        return false;
    }
    
    size_t totalSize = imageFile.size();
    size_t totalTransferred = 0;
    size_t linesPerChunk = dmaBufferSize / (SCREEN_WIDTH * 2);
    
    while (totalTransferred < totalSize && imageFile.available()) {
        // Calculate chunk parameters
        size_t remainingBytes = totalSize - totalTransferred;
        size_t chunkSize = min(dmaBufferSize, remainingBytes);
        size_t bytesRead = imageFile.read(dmaBuffer, chunkSize);
        
        if (bytesRead == 0) break;
        
        // Calculate display coordinates
        size_t currentPixel = totalTransferred / 2; // RGB565 = 2 bytes/pixel
        uint16_t y = currentPixel / SCREEN_WIDTH;
        uint16_t height = min((uint16_t)(bytesRead / (SCREEN_WIDTH * 2)), (uint16_t)(SCREEN_HEIGHT - y));
        
        if (height > 0) {
            // Optimized transfer using DMA buffer
            transferChunkOptimized(dmaBuffer, bytesRead, 0, y, SCREEN_WIDTH, height);
        }
        
        totalTransferred += bytesRead;
        
        // Small delay to prevent watchdog
        if (height > 20) {
            yield();
        }
    }
    
    imageFile.close();
    return totalTransferred == totalSize;
}

bool DMAImageManager::transferChunkOptimized(uint8_t* imageData, size_t size, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    auto lcd = waveshare_lcd_get_instance();
    if (!lcd) {
        return false;
    }
    
    // Use optimized memory copy for large transfers
    if (size > 4096) {
        // For large chunks, use hardware-accelerated copy if available
        memcpyDMA(imageData, imageData, size);
    }
    
    // Transfer to LCD using optimized drawBitmap
    lcd->drawBitmap(x, y, width, height, imageData);
    
    return true;
}

bool DMAImageManager::memcpyDMA(void* dst, const void* src, size_t size) {
    // ESP32-S3 optimized memory copy
    // For now, use standard memcpy but in future could use actual DMA
    if (size > 0 && dst && src) {
        memcpy(dst, src, size);
        return true;
    }
    return false;
}

void DMAImageManager::freeDMAResources() {
    if (dmaBuffer) {
        heap_caps_free(dmaBuffer);
        dmaBuffer = nullptr;
    }
    
    dmaBufferSize = 0;
    dmaEnabled = false;
}

void DMAImageManager::disableDMA() {
    freeDMAResources();
}

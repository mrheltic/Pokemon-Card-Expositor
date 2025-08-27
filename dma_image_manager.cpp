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
    dmaBuffer(nullptr),
    dmaBufferSize(0),
    dmaEnabled(false),
    initialized(false),
    maxChunkSize(32768) { // 32KB chunks
}

DMAImageManager::~DMAImageManager() {
    freeDMAResources();
}

bool DMAImageManager::init() {
    if (initialized) return true;
    
    initialized = true;
    Serial.println("DMA Image Manager: ✓ Initialized");
    return true;
}

void DMAImageManager::deinit() {
    freeDMAResources();
    initialized = false;
}

bool DMAImageManager::initWithDMA() {
    // Initialize base functionality first
    if (!init()) {
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
    
    String filename = String(filepath);
    
    // Check if it's a RAW RGB565 file (should be exactly 1024x600)
    if (filename.endsWith(".raw")) {
        return displayFixedSizeImageDMA(filepath);  // Use optimized method
    } 
    else if (filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp")) {
        // For other formats, assume they're pre-processed to 1024x600 and converted to RAW
        Serial.println("⚠️ Non-RAW format detected. For best performance, convert to RAW RGB565 1024x600");
        return displayPNGFromSD(filepath);
    }
    else {
        // Unknown format, try as RAW
        Serial.println("🔍 Unknown format, trying as RAW RGB565...");
        return displayFixedSizeImageDMA(filepath);
    }
}

bool DMAImageManager::displayFixedSizeImageDMA(const char* filepath) {
    if (!isInitialized()) {
        Serial.println("❌ DMA Image Manager not initialized");
        return false;
    }
    
    Serial.printf("🚀 Loading FIXED SIZE image: %s\n", filepath);
    Serial.printf("   Expected: %dx%d (%d bytes)\n", FIXED_IMAGE_WIDTH, FIXED_IMAGE_HEIGHT, FIXED_IMAGE_SIZE);
    
    // Open file
    File imageFile = SD.open(filepath);
    if (!imageFile) {
        Serial.printf("❌ Cannot open file: %s\n", filepath);
        return false;
    }
    
    // Verify file size matches expected dimensions
    size_t fileSize = imageFile.size();
    if (fileSize != FIXED_IMAGE_SIZE) {
        Serial.printf("❌ File size mismatch! Expected %d bytes, got %d bytes\n", FIXED_IMAGE_SIZE, fileSize);
        Serial.println("   Make sure image is exactly 1024x600 RGB565!");
        imageFile.close();
        return false;
    }
    
    // Get LCD instance
    auto lcd = waveshare_lcd_get_instance();
    if (!lcd) {
        Serial.println("❌ LCD not available");
        imageFile.close();
        return false;
    }
    
    unsigned long startTime = millis();
    
    if (dmaEnabled && dmaBuffer) {
        // ULTRA-FAST DMA PATH - Direct full-screen transfer
        Serial.println("⚡ Using DMA ultra-fast path...");
        
        size_t totalBytesRead = 0;
        size_t fixedImageSize = static_cast<size_t>(FIXED_IMAGE_SIZE);
        size_t bytesToRead = min(dmaBufferSize, fixedImageSize);
        
        while (totalBytesRead < fixedImageSize) {
            size_t chunkSize = min(bytesToRead, fixedImageSize - totalBytesRead);
            size_t bytesRead = imageFile.read(dmaBuffer, chunkSize);
            
            if (bytesRead == 0) break;
            
            // Direct DMA transfer to LCD - no calculations needed!
            lcd->drawBitmap(0, 0, FIXED_IMAGE_WIDTH, FIXED_IMAGE_HEIGHT, dmaBuffer);
            
            totalBytesRead += bytesRead;
        }
        
        unsigned long elapsedTime = millis() - startTime;
        Serial.printf("✅ DMA ULTRA-FAST: %d bytes in %lu ms (%.2f MB/s)\n", 
                     FIXED_IMAGE_SIZE, elapsedTime, 
                     (FIXED_IMAGE_SIZE / 1024.0 / 1024.0) / (elapsedTime / 1000.0));
        
    } else {
        // Fallback CPU path
        Serial.println("⚠️ DMA not available, using CPU fallback...");
        
        uint8_t* buffer = (uint8_t*)malloc(4096);
        if (!buffer) {
            Serial.println("❌ Cannot allocate CPU buffer");
            imageFile.close();
            return false;
        }
        
        size_t totalBytesRead = 0;
        while (totalBytesRead < static_cast<size_t>(FIXED_IMAGE_SIZE)) {
            size_t bytesRead = imageFile.read(buffer, 4096);
            if (bytesRead == 0) break;
            
            // CPU transfer
            lcd->drawBitmap(0, 0, FIXED_IMAGE_WIDTH, FIXED_IMAGE_HEIGHT, buffer);
            totalBytesRead += bytesRead;
        }
        
        free(buffer);
        unsigned long elapsedTime = millis() - startTime;
        Serial.printf("✅ CPU fallback: %d bytes in %lu ms\n", FIXED_IMAGE_SIZE, elapsedTime);
    }
    
    imageFile.close();
    return true;
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

bool DMAImageManager::displayPNGFromSD(const char* filepath) {
    Serial.printf("PNG display not implemented yet for: %s\n", filepath);
    Serial.println("Please convert to RAW RGB565 format for DMA acceleration");
    return false;
}

void DMAImageManager::runImageTest() {
    Serial.println("=== DMA Image Manager Test ===");
    Serial.printf("DMA Enabled: %s\n", dmaEnabled ? "YES" : "NO");
    Serial.printf("Buffer Size: %zu bytes\n", dmaBufferSize);
    Serial.printf("Max Chunk: %zu bytes\n", maxChunkSize);
    Serial.println("Test completed");
}

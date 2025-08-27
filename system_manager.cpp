#include "system_manager.h"
#include "project_config.h"
#include "dma_image_manager.h"
#include "image_browser.h"

SystemManager systemManager;

SystemManager::SystemManager() : systemInitialized(false) {
}

SystemManager::~SystemManager() {
    shutdownSystem();
}

bool SystemManager::initializeSystem() {
    if (systemInitialized) {
        return true;
    }
    
    // Initialize components in dependency order
    if (!initializeSD() || !initializeLCD() || !initializeImage() || !initializeBrowser()) {
        shutdownSystem();
        return false;
    }
    
    // Initialize brightness control (optional)
    initializeBrightness();
    
    systemInitialized = true;
    return true;
}

bool SystemManager::initializeSD() {
    if (!sdManager.init()) {
        Serial.println("ERROR: SD Manager failed to initialize");
        return false;
    }
    return true;
}

bool SystemManager::initializeLCD() {
    if (!lcdManager.init()) {
        Serial.println("ERROR: LCD Manager failed to initialize");
        return false;
    }
    return true;
}

bool SystemManager::initializeImage() {
    // Initialize DMA image manager
    if (dmaImageManager.initWithDMA()) {
        return true;
    }
    
    Serial.println("ERROR: DMA Image Manager failed to initialize");
    return false;
}
bool SystemManager::initializeBrowser() {
    if (!imageBrowser.init()) {
        Serial.println("ERROR: Image Browser failed to initialize");
        return false;
    }
    return true;
}

bool SystemManager::initializeBrightness() {
    if (!brightnessManager.init()) {
        Serial.println("WARNING: Brightness Manager failed to initialize");
        return false; // Non è critico per il sistema
    }
    return true;
}

void SystemManager::shutdownSystem() {
    if (systemInitialized) {
        Serial.println("=== System Shutdown ===");
        
        // Shutdown image manager
        dmaImageManager.deinit();
        brightnessManager.deinit();
        lcdManager.deinit();
        sdManager.deinit();
        
        systemInitialized = false;
        Serial.println("System shutdown complete");
    }
}

void SystemManager::printSystemStatus() {
    Serial.println("\n=== System Status ===");
    Serial.printf("System Ready: %s\n", systemInitialized ? "✓ YES" : "✗ NO");
    Serial.printf("SD Manager: %s\n", sdManager.isInitialized() ? "✓ READY" : "✗ NOT READY");
    Serial.printf("LCD Manager: %s\n", lcdManager.isInitialized() ? "✓ READY" : "✗ NOT READY");
    Serial.printf("DMA Manager: %s\n", dmaImageManager.isInitialized() ? "✓ READY" : "✗ NOT READY");
    if (dmaImageManager.isInitialized()) {
        Serial.printf("DMA Mode: %s\n", dmaImageManager.isDMAEnabled() ? "⚡ ENABLED" : "🐌 DISABLED");
    }
    Serial.printf("Brightness: %s\n", brightnessManager.isReady() ? "✓ READY" : "✗ NOT READY");
    if (brightnessManager.isReady()) {
        Serial.printf("Current Level: %d%%\n", brightnessManager.getBrightness());
    }
    Serial.println("=====================\n");
}

void SystemManager::runSDTest() {
    Serial.println("\n--- SD Subsystem Test ---");
    sdManager.runSDTest();
    Serial.println("--- Test Complete ---\n");
}

void SystemManager::runLCDTest() {
    Serial.println("\n--- LCD Subsystem Test ---");
    lcdManager.runLCDTest();
    Serial.println("--- Test Complete ---\n");
}

void SystemManager::runImageTest() {
    Serial.println("\n--- DMA Image Subsystem Test ---");
    
    if (dmaImageManager.isInitialized()) {
        Serial.println("Testing DMA Image Manager...");
        dmaImageManager.runImageTest();
    } else {
        Serial.println("DMA Image Manager not initialized");
    }
    
    Serial.println("--- Test Complete ---\n");
}

void SystemManager::displayImage(const char* filepath) {
    if (!systemInitialized) {
        return;
    }
    
    // Use DMA manager for image display
    if (dmaImageManager.isInitialized()) {
        dmaImageManager.displayImageDMA(filepath);
    } else {
        Serial.println("ERROR: DMA Image Manager not available");
    }
}

void SystemManager::displayCurrentImage() {
    if (!systemInitialized || !imageBrowser.hasImages()) {
        return;
    }
    
    String currentPath = imageBrowser.getCurrentImagePath();
    if (currentPath.length() > 0) {
        displayImage(currentPath.c_str());
    }
}

void SystemManager::nextImage() {
    if (!systemInitialized) {
        return;
    }
    
    if (imageBrowser.nextImage()) {
        displayCurrentImage();
    }
}

void SystemManager::previousImage() {
    if (!systemInitialized) {
        return;
    }
    
    if (imageBrowser.previousImage()) {
        displayCurrentImage();
    }
}

void SystemManager::listImages() {
    if (!systemInitialized) {
        return;
    }
    
    imageBrowser.printImageList();
}

void SystemManager::runAllTests() {
    if (!systemInitialized) {
        Serial.println("ERROR: System not initialized - cannot run tests");
        return;
    }
    
    Serial.println("\n=== Running All System Tests ===");
    runSDTest();
    runLCDTest();
    runImageTest();
    Serial.println("=== All Tests Complete ===\n");
}

void SystemManager::update() {
    // Main system update loop - called from Arduino loop()
    // Touch functionality removed to prevent I2C conflicts
}

// Brightness control methods
void SystemManager::setBrightness(int percentage) {
    brightnessManager.setBrightness(percentage);
}

void SystemManager::increaseBrightness() {
    brightnessManager.increaseBrightness();
}

void SystemManager::decreaseBrightness() {
    brightnessManager.decreaseBrightness();
}

void SystemManager::setBrightnessLow() {
    brightnessManager.setLow();
}

void SystemManager::setBrightnessMedium() {
    brightnessManager.setMedium();
}

void SystemManager::setBrightnessHigh() {
    brightnessManager.setHigh();
}

void SystemManager::setBrightnessMax() {
    brightnessManager.setMax();
}

void SystemManager::setBrightnessNight() {
    brightnessManager.setNight();
}
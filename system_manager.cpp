#include "system_manager.h"
#include "project_config.h"
#include "dma_image_manager.h"

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
    if (!initializeSD() || !initializeLCD() || !initializeImage()) {
        shutdownSystem();
        return false;
    }
    
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
    // Try DMA-enhanced image manager first
    if (dmaImageManager.initWithDMA()) {
        return true;
    }
    
    // Fallback to standard image manager
    if (!imageManager.init()) {
        Serial.println("ERROR: Image Manager failed to initialize");
        return false;
    }
    return true;
}

void SystemManager::shutdownSystem() {
    if (systemInitialized) {
        Serial.println("=== System Shutdown ===");
        
        // Shutdown both image managers
        dmaImageManager.deinit();
        imageManager.deinit();
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
    Serial.printf("Image Manager: %s\n", imageManager.isInitialized() ? "✓ READY" : "✗ NOT READY");
    Serial.printf("DMA Manager: %s\n", dmaImageManager.isInitialized() ? "✓ READY" : "✗ NOT READY");
    if (dmaImageManager.isInitialized()) {
        Serial.printf("DMA Mode: %s\n", dmaImageManager.isDMAEnabled() ? "⚡ ENABLED" : "🐌 DISABLED");
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
    Serial.println("\n--- Enhanced Image Subsystem Test ---");
    
    if (dmaImageManager.isInitialized()) {
        Serial.println("Testing DMA Image Manager...");
        dmaImageManager.runImageTest();
    } else {
        Serial.println("Testing Standard Image Manager...");
        imageManager.runImageTest();
    }
    
    Serial.println("--- Test Complete ---\n");
}

void SystemManager::displayImage(const char* filepath) {
    if (!systemInitialized) {
        return;
    }
    
    // Use DMA manager if available, otherwise fallback to standard
    if (dmaImageManager.isInitialized()) {
        dmaImageManager.displayImageDMA(filepath);
    } else {
        imageManager.displayPNGFromSD(filepath);
    }
}

void SystemManager::listImages() {
    if (!systemInitialized) {
        Serial.println("ERROR: System not initialized");
        return;
    }
    
    // Use whichever image manager is available
    if (dmaImageManager.isInitialized()) {
        dmaImageManager.listImagesOnSD();
    } else {
        imageManager.listImagesOnSD();
    }
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
    // Currently minimal - could add periodic tasks here if needed
}
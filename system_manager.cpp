#include "system_manager.h"
#include "project_config.h"

SystemManager systemManager;

SystemManager::SystemManager() : systemInitialized(false) {
}

SystemManager::~SystemManager() {
    shutdownSystem();
}

bool SystemManager::initializeSystem() {
    if (systemInitialized) {
        Serial.println("System already initialized");
        return true;
    }
    
    Serial.println("=== System Initialization ===");
    
    // Initialize components in dependency order
    if (!initializeSD() || !initializeLCD() || !initializeImage()) {
        shutdownSystem();
        return false;
    }
    
    systemInitialized = true;
    Serial.println("=== System Ready ===");
    printSystemStatus();
    
    return true;
}

bool SystemManager::initializeSD() {
    Serial.println("Initializing SD subsystem...");
    if (!sdManager.init()) {
        Serial.println("ERROR: SD Manager failed to initialize");
        return false;
    }
    Serial.println("✓ SD subsystem ready");
    return true;
}

bool SystemManager::initializeLCD() {
    Serial.println("Initializing LCD subsystem...");
    if (!lcdManager.init()) {
        Serial.println("ERROR: LCD Manager failed to initialize");
        return false;
    }
    Serial.println("✓ LCD subsystem ready");
    return true;
}

bool SystemManager::initializeImage() {
    Serial.println("Initializing Image subsystem...");
    if (!imageManager.init()) {
        Serial.println("ERROR: Image Manager failed to initialize");
        return false;
    }
    Serial.println("✓ Image subsystem ready");
    return true;
}

void SystemManager::shutdownSystem() {
    if (systemInitialized) {
        Serial.println("=== System Shutdown ===");
        
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
    Serial.println("\n--- Image Subsystem Test ---");
    imageManager.runImageTest();
    Serial.println("--- Test Complete ---\n");
}

void SystemManager::displayImage(const char* filepath) {
    if (!systemInitialized) {
        Serial.println("ERROR: System not initialized");
        return;
    }
    
    Serial.printf("Displaying image: %s\n", filepath);
    imageManager.displayPNGFromSD(filepath);
}

void SystemManager::listImages() {
    if (!systemInitialized) {
        Serial.println("ERROR: System not initialized");
        return;
    }
    
    imageManager.listImagesOnSD();
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
#include "system_manager.h"
#include "project_config.h"
#include "dma_image_manager.h"
#include "image_browser.h"

// Forward declarations for global manager instances
extern SDManager sdManager;
extern LCDManager lcdManager;
extern DMAImageManager dmaImageManager;
extern ImageBrowser imageBrowser;
extern BrightnessManager brightnessManager;

#if ENABLE_WIFI_CONFIG
extern WiFiConfigManager wifiConfigManager;
#endif

SystemManager systemManager;
SystemManager* g_systemManager = &systemManager;  // Global reference for web interface

SystemManager::SystemManager() : systemInitialized(false), slideshowActive(false), 
                                 lastSlideshowUpdate(0), currentImageIndex(0) {
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
    
    // Initialize image browser (optional - not critical for WiFi functionality)
    if (!initializeBrowser()) {
        Serial.println("[SystemManager] WARNING: Image Browser not available, but system will continue");
    }
    
    // Initialize brightness control (optional)
    initializeBrightness();
    
    // Initialize WiFi config manager (optional)
    #if ENABLE_WIFI_CONFIG
    initializeWiFi();
    #endif
    
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
    // Create images directory if it doesn't exist
    if (!SD.exists("/images")) {
        if (SD.mkdir("/images")) {
            Serial.println("[SystemManager] Created /images directory");
        } else {
            Serial.println("[SystemManager] WARNING: Failed to create /images directory");
        }
    }
    
    if (!imageBrowser.init()) {
        Serial.println("ERROR: Image Browser failed to initialize");
        Serial.println("HINT: Make sure you have images in /images/ folder or SD card root");
        Serial.println("Supported formats: .png, .jpg, .jpeg, .bmp, .raw");
        return false;
    }
    Serial.printf("[SystemManager] ✅ Image Browser initialized with %d images\n", imageBrowser.getImageCount());
    return true;
}

bool SystemManager::initializeBrightness() {
    if (!brightnessManager.init()) {
        Serial.println("WARNING: Brightness Manager failed to initialize");
        return false; // Non è critico per il sistema
    }
    return true;
}

bool SystemManager::initializeWiFi() {
    #if ENABLE_WIFI_CONFIG
    if (!wifiConfigManager.initialize()) {
        Serial.println("WARNING: WiFi Config Manager failed to initialize");
        return false; // Non è critico per il sistema
    }
    
    // Start slideshow if auto-start is enabled
    if (wifiConfigManager.getSlideshowConfig().autoStart) {
        startSlideshow();
    }
    
    return true;
    #else
    return false;
    #endif
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
    
    #if ENABLE_WIFI_CONFIG
    Serial.printf("WiFi: %s\n", wifiConfigManager.isWiFiConnected() ? "✓ CONNECTED" : "✗ DISCONNECTED");
    if (wifiConfigManager.isWiFiConnected()) {
        Serial.printf("Local IP: %s\n", wifiConfigManager.getLocalIP().c_str());
    }
    Serial.printf("Web Server: %s\n", wifiConfigManager.isWebServerRunning() ? "✓ RUNNING" : "✗ STOPPED");
    Serial.printf("Slideshow: %s\n", slideshowActive ? "✓ ACTIVE" : "✗ STOPPED");
    Serial.printf("Web Interface: %s\n", getWebInterfaceURL().c_str());
    #endif
    
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
    
    #if ENABLE_WIFI_CONFIG
    // Handle web server requests
    wifiConfigManager.handleWebRequests();
    
    // Update slideshow if active
    updateSlideshow();
    #endif
}

// WiFi and Slideshow methods
bool SystemManager::startSlideshow() {
    #if ENABLE_WIFI_CONFIG && ENABLE_SLIDESHOW
    const SlideshowConfig& config = wifiConfigManager.getSlideshowConfig();
    if (!config.enabled) {
        Serial.println("[Slideshow] Cannot start - slideshow is disabled in config");
        return false;
    }
    
    slideshowActive = true;
    lastSlideshowUpdate = millis();
    
    // Set brightness according to config; if disabled, force 0
    if (config.disableBrightness) {
        setBrightness(0);
    } else {
        setBrightness((config.brightness * 100) / 255);
    }
    
    Serial.println("[Slideshow] Started");
    return true;
    #else
    Serial.println("[Slideshow] Not available - WiFi/Slideshow disabled");
    return false;
    #endif
}

bool SystemManager::stopSlideshow() {
    #if ENABLE_WIFI_CONFIG && ENABLE_SLIDESHOW
    slideshowActive = false;
    Serial.println("[Slideshow] Stopped");
    return true;
    #else
    return false;
    #endif
}

bool SystemManager::pauseSlideshow() {
    #if ENABLE_WIFI_CONFIG && ENABLE_SLIDESHOW
    slideshowActive = false;
    Serial.println("[Slideshow] Paused");
    return true;
    #else
    return false;
    #endif
}

void SystemManager::updateSlideshow() {
    #if ENABLE_WIFI_CONFIG && ENABLE_SLIDESHOW
    if (!slideshowActive) return;
    
    const SlideshowConfig& config = wifiConfigManager.getSlideshowConfig();
    unsigned long currentTime = millis();
    
    if (currentTime - lastSlideshowUpdate >= config.intervalMs) {
        if (config.randomOrder) {
            // Random image selection
            int totalImages = imageBrowser.getImageCount();
            if (totalImages > 0) {
                currentImageIndex = random(totalImages);
                imageBrowser.goToIndex(currentImageIndex);
            }
        } else {
            // Sequential image selection
            nextImage();
        }
        
        lastSlideshowUpdate = currentTime;
        
        // Check if we should loop
        if (!config.loop && currentImageIndex >= imageBrowser.getImageCount() - 1) {
            stopSlideshow();
        }
    }
    #endif
}

bool SystemManager::isWiFiConnected() const {
    #if ENABLE_WIFI_CONFIG
    return wifiConfigManager.isWiFiConnected();
    #else
    return false;
    #endif
}

String SystemManager::getWebInterfaceURL() const {
    #if ENABLE_WIFI_CONFIG
    String ip = wifiConfigManager.getLocalIP();
    if (ip.isEmpty()) {
        ip = wifiConfigManager.getAPIP();
    }
    return "http://" + ip + "/";
    #else
    return "Web interface disabled";
    #endif
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
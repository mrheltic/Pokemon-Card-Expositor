#ifndef __SYSTEM_MANAGER_H
#define __SYSTEM_MANAGER_H

#include "sd_manager.h"
#include "lcd_manager.h"
#include "dma_image_manager.h"
#include "image_browser.h"
#include "brightness_manager.h"
#include "wifi_config_manager.h"

class SystemManager {
private:
    bool systemInitialized;
    bool slideshowActive;
    unsigned long lastSlideshowUpdate;
    int currentImageIndex;
    
    // Initialization helpers
    bool initializeSD();
    bool initializeLCD();
    bool initializeImage();
    bool initializeBrowser();
    bool initializeBrightness();
    bool initializeWiFi();

public:
    SystemManager();
    ~SystemManager();
    
    // System control
    bool initializeSystem();
    void shutdownSystem();
    bool isSystemReady() const { return systemInitialized; }
    
    // System tests
    void runAllTests();
    void runSDTest();
    void runLCDTest();
    void runImageTest();
    
    // Image functions
    void displayImage(const char* filepath);
    void displayCurrentImage();
    void nextImage();
    void previousImage();
    void listImages();
    
    // Brightness control
    void setBrightness(int percentage);
    void increaseBrightness();
    void decreaseBrightness();
    void setBrightnessLow();
    void setBrightnessMedium();
    void setBrightnessHigh();
    void setBrightnessMax();
    void setBrightnessNight();
    
    // WiFi and Slideshow control
    bool startSlideshow();
    bool stopSlideshow();
    bool pauseSlideshow();
    void updateSlideshow();
    bool isWiFiConnected() const;
    String getWebInterfaceURL() const;
    
    // System status
    void printSystemStatus();
    
    // Main loop function
    void update();
};

// Global instance for web interface callbacks
extern SystemManager* g_systemManager;

extern SystemManager systemManager;

#endif

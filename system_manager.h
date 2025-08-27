#ifndef __SYSTEM_MANAGER_H
#define __SYSTEM_MANAGER_H

#include "sd_manager.h"
#include "lcd_manager.h"
#include "dma_image_manager.h"
#include "image_browser.h"
#include "brightness_manager.h"

class SystemManager {
private:
    bool systemInitialized;
    
    // Initialization helpers
    bool initializeSD();
    bool initializeLCD();
    bool initializeImage();
    bool initializeBrowser();
    bool initializeBrightness();

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
    
    // System status
    void printSystemStatus();
    
    // Main loop function
    void update();
};

extern SystemManager systemManager;

#endif

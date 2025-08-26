#ifndef __SYSTEM_MANAGER_H
#define __SYSTEM_MANAGER_H

#include "sd_manager.h"
#include "lcd_manager.h"
#include "image_manager.h"

class SystemManager {
private:
    bool systemInitialized;
    
    // Initialization helpers
    bool initializeSD();
    bool initializeLCD();
    bool initializeImage();

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
    void listImages();
    
    // System status
    void printSystemStatus();
    
    // Main loop function
    void update();
};

extern SystemManager systemManager;

#endif

#ifndef __SD_MANAGER_H
#define __SD_MANAGER_H

#include "waveshare_sd_card.h"

class SDManager {
private:
    esp_expander::CH422G *expander;
    bool initialized;

public:
    SDManager();
    ~SDManager();
    
    bool init();
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Test functions
    void runSDTest();
    void printCardInfo();
    void runFileSystemTest();
    
    // Utility functions
    esp_expander::CH422G* getExpander() { return expander; }
};

extern SDManager sdManager;

#endif

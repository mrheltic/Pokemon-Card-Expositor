#ifndef __LCD_MANAGER_H
#define __LCD_MANAGER_H

#include "waveshare_lcd_port.h"

class LCDManager {
private:
    bool initialized;

public:
    LCDManager();
    ~LCDManager();
    
    bool init();
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Test functions
    void runLCDTest();
    void drawColorBar();
    void calibrateDisplay();
    void fillScreen(uint16_t color);
    
    // Utility functions
    void setBacklight(bool state);
    void clearScreen();
    void displayMessage(const char* message);
};

extern LCDManager lcdManager;

#endif

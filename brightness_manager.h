#ifndef __BRIGHTNESS_MANAGER_H
#define __BRIGHTNESS_MANAGER_H

#include <Arduino.h>

class BrightnessManager {
private:
    static const int BACKLIGHT_PIN = 2;        // Pin backlight LCD
    static const int PWM_FREQUENCY = 5000;     // Frequenza PWM (5kHz)
    static const int PWM_RESOLUTION = 8;       // Risoluzione 8-bit (0-255)
    
    int currentBrightness;                     // Luminosità corrente (0-100%)
    bool isInitialized;
    
public:
    BrightnessManager();
    ~BrightnessManager();
    
    // Initialization
    bool init();
    void deinit();
    bool isReady() const { return isInitialized; }
    
    // Brightness control
    void setBrightness(int percentage);        // 0-100%
    void increaseBrightness(int step = 10);    // Aumenta luminosità
    void decreaseBrightness(int step = 10);    // Diminuisce luminosità
    int getBrightness() const { return currentBrightness; }
    
    // Presets
    void setLow();          // 25%
    void setMedium();       // 50%
    void setHigh();         // 75%
    void setMax();          // 100%
    void setNight();        // 10%
    
    // Power saving
    void turnOff();         // Spegne backlight
    void turnOn();          // Accende con ultima luminosità
    
    // Status
    void printStatus();
};

extern BrightnessManager brightnessManager;

#endif

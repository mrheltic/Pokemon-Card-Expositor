#include "brightness_manager.h"

BrightnessManager brightnessManager;

BrightnessManager::BrightnessManager() {
    currentBrightness = 75;  // Default 75%
    isInitialized = false;
}

BrightnessManager::~BrightnessManager() {
    deinit();
}

bool BrightnessManager::init() {
    if (isInitialized) {
        return true;
    }
    
    Serial.println("🔆 Initializing Brightness Manager...");
    
    // Configura pin backlight come PWM (nuova API ESP32)
    if (!ledcAttach(BACKLIGHT_PIN, PWM_FREQUENCY, PWM_RESOLUTION)) {
        Serial.println("❌ Failed to setup PWM channel");
        return false;
    }
    
    // Imposta luminosità iniziale
    setBrightness(currentBrightness);
    
    isInitialized = true;
    Serial.printf("✅ Brightness Manager ready - Default: %d%%\n", currentBrightness);
    
    return true;
}

void BrightnessManager::deinit() {
    if (isInitialized) {
        ledcDetach(BACKLIGHT_PIN);
        isInitialized = false;
        Serial.println("🔆 Brightness Manager shutdown");
    }
}

void BrightnessManager::setBrightness(int percentage) {
    if (!isInitialized) {
        Serial.println("❌ Brightness Manager not initialized");
        return;
    }
    
    // Limita valori 0-100%
    percentage = constrain(percentage, 0, 100);
    
    // Converte percentuale in valore PWM (0-255)
    int pwmValue = map(percentage, 0, 100, 0, 255);
    
    // Applica PWM (nuova API)
    ledcWrite(BACKLIGHT_PIN, pwmValue);
    
    currentBrightness = percentage;
    
    Serial.printf("🔆 Brightness set to %d%% (PWM: %d)\n", percentage, pwmValue);
}

void BrightnessManager::increaseBrightness(int step) {
    setBrightness(currentBrightness + step);
}

void BrightnessManager::decreaseBrightness(int step) {
    setBrightness(currentBrightness - step);
}

void BrightnessManager::setLow() {
    setBrightness(25);
    Serial.println("🔅 Low brightness mode");
}

void BrightnessManager::setMedium() {
    setBrightness(50);
    Serial.println("🔆 Medium brightness mode");
}

void BrightnessManager::setHigh() {
    setBrightness(75);
    Serial.println("🔆 High brightness mode");
}

void BrightnessManager::setMax() {
    setBrightness(100);
    Serial.println("☀️ Maximum brightness mode");
}

void BrightnessManager::setNight() {
    setBrightness(10);
    Serial.println("🌙 Night mode brightness");
}

void BrightnessManager::turnOff() {
    if (!isInitialized) return;
    
    ledcWrite(BACKLIGHT_PIN, 0);
    Serial.println("📴 Backlight OFF");
}

void BrightnessManager::turnOn() {
    if (!isInitialized) return;
    
    setBrightness(currentBrightness);
    Serial.println("💡 Backlight ON");
}

void BrightnessManager::printStatus() {
    Serial.println("\n🔆 === BRIGHTNESS STATUS ===");
    Serial.printf("Status: %s\n", isInitialized ? "✅ READY" : "❌ NOT READY");
    Serial.printf("Current: %d%%\n", currentBrightness);
    Serial.printf("PWM Pin: %d\n", BACKLIGHT_PIN);
    Serial.printf("PWM Frequency: %dHz\n", PWM_FREQUENCY);
    Serial.println("==========================\n");
}

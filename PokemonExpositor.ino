/*
 * Pokemon Expositor - Main Firmware
 * 
 * Hardware: Waveshare ESP32-S3 RGB LCD + SD Card
 * Features: Pokemon card display with rotation and full-screen support
 */

#include <Arduino.h>
#include "project_config.h"
#include "system_manager.h"

bool systemReady = false;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(STARTUP_DELAY_MS);
    
    // Initialize system components
    systemReady = systemManager.initializeSystem();
    
    if (systemReady) {
        // Display first image from browser
        systemManager.displayCurrentImage();
    }
}

void loop() {
    handleSerialCommands();
    
    if (systemReady) {
        systemManager.update();
    }
    
    delay(MAIN_LOOP_DELAY_MS);
}

void printAvailableCommands() {
    Serial.println("\nAvailable commands:");
    Serial.println("  'show' - Display Pokemon image (rotated full-screen)");
    Serial.println("  'next' - Next image");
    Serial.println("  'prev' - Previous image");
    Serial.println("  'current' - Show current image");
    Serial.println("  'clear' - Clear screen");
    Serial.println("  'list' - List images on SD card");
    Serial.println("  'test' - Run system tests");
    Serial.println("  'status' - Show system status");
    Serial.println("  'help' - Show all commands");
    Serial.println();
}

void handleSerialCommands() {
    if (!Serial.available()) return;
    
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command.length() == 0) return;
    
    Serial.printf("Command: '%s'\n", command.c_str());
    
    if (!systemReady && command != "help" && command != "status") {
        Serial.println("System not ready");
        return;
    }
    
    executeCommand(command);
    Serial.println();
}

void executeCommand(const String& command) {
    // Image commands
    if (command == "show") {
        systemManager.displayImage(DEFAULT_IMAGE_PATH);
    }
    else if (command == "show-normal") {
        systemManager.displayImage(FALLBACK_IMAGE_PATH);
    }
    else if (command == "show-original") {
        systemManager.displayImage(ORIGINAL_PNG_PATH);
    }
    
    // Navigation commands
    else if (command == "next") {
        systemManager.nextImage();
    }
    else if (command == "prev" || command == "previous") {
        systemManager.previousImage();
    }
    else if (command == "current") {
        systemManager.displayCurrentImage();
    }
    
    // Brightness commands
    else if (command == "bright+" || command == "brightness+") {
        systemManager.increaseBrightness();
    }
    else if (command == "bright-" || command == "brightness-") {
        systemManager.decreaseBrightness();
    }
    else if (command == "bright low" || command == "brightness low") {
        systemManager.setBrightnessLow();
    }
    else if (command == "bright med" || command == "bright medium" || command == "brightness medium") {
        systemManager.setBrightnessMedium();
    }
    else if (command == "bright high" || command == "brightness high") {
        systemManager.setBrightnessHigh();
    }
    else if (command == "bright max" || command == "brightness max") {
        systemManager.setBrightnessMax();
    }
    else if (command == "bright night" || command == "brightness night") {
        systemManager.setBrightnessNight();
    }
    else if (command.startsWith("bright ") || command.startsWith("brightness ")) {
        // Extract number from "bright 50" or "brightness 50"
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex > 0) {
            String numberStr = command.substring(spaceIndex + 1);
            int brightness = numberStr.toInt();
            if (brightness >= 0 && brightness <= 100) {
                systemManager.setBrightness(brightness);
            } else {
                Serial.println("❌ Brightness must be 0-100%");
            }
        }
    }
    
    // System commands
    else if (command == "clear") {
        lcdManager.clearScreen();
    }
    else if (command == "clear-top") {
        clearTopArea();
    }
    else if (command == "list") {
        systemManager.listImages();
    }
    else if (command == "images") {
        systemManager.listImages();
    }
    
    // Test commands
    else if (command == "test") {
        systemManager.runAllTests();
    }
    else if (command == "sd") {
        systemManager.runSDTest();
    }
    else if (command == "lcd") {
        systemManager.runLCDTest();
    }
    else if (command == "image") {
        systemManager.runImageTest();
    }
    
    // Utility commands
    else if (command == "calibrate") {
        lcdManager.calibrateDisplay();
    }
    else if (command == "colorbar") {
        lcdManager.drawColorBar();
    }
    else if (command == "status") {
        systemManager.printSystemStatus();
    }
    else if (command == "restart") {
        Serial.println("Restarting system...");
        ESP.restart();
    }
    else if (command == "help") {
        printFullHelp();
    }
    else {
        Serial.printf("Unknown command: '%s'\n", command.c_str());
        Serial.println("Type 'help' for available commands.");
    }
}

void clearTopArea() {
    auto lcd = waveshare_lcd_get_instance();
    if (lcd) {
        uint8_t* clearBuffer = (uint8_t*)malloc(1024 * 2 * 50);
        if (clearBuffer) {
            memset(clearBuffer, 0, 1024 * 2 * 50);
            lcd->drawBitmap(0, 0, 1024, 50, clearBuffer);
            free(clearBuffer);
        }
    }
}

void printFullHelp() {
    Serial.println("=== Pokemon Expositor Commands ===");
    Serial.println("IMAGE NAVIGATION:");
    Serial.println("  next         - Next image");
    Serial.println("  prev         - Previous image");
    Serial.println("  current      - Show current image");
    Serial.println("  list/images  - List all images");
    Serial.println();
    Serial.println("BRIGHTNESS CONTROL:");
    Serial.println("  bright+      - Increase brightness (+10%)");
    Serial.println("  bright-      - Decrease brightness (-10%)");
    Serial.println("  bright low   - Low brightness (25%)");
    Serial.println("  bright med   - Medium brightness (50%)");
    Serial.println("  bright high  - High brightness (75%)");
    Serial.println("  bright max   - Maximum brightness (100%)");
    Serial.println("  bright night - Night mode (10%)");
    Serial.println("  bright 50    - Set specific brightness (0-100%)");
    Serial.println();
    Serial.println("IMAGE DISPLAY:");
    Serial.println("  show         - Display rotated full-screen Pokemon");
    Serial.println("  show-normal  - Display normal centered Pokemon");
    Serial.println("  show-original- Display original PNG");
    Serial.println();
    Serial.println("SYSTEM CONTROL:");
    Serial.println("  clear        - Clear screen");
    Serial.println("  clear-top    - Clear top area (fix glitches)");
    Serial.println("  status       - Show system status");
    Serial.println("  restart      - Restart system");
    Serial.println();
    Serial.println("TESTING:");
    Serial.println("  test         - Run all tests");
    Serial.println("  sd           - Test SD card");
    Serial.println("  lcd          - Test LCD display");
    Serial.println("  image        - Test image system");
    Serial.println();
    Serial.println("ADVANCED:");
    Serial.println("  calibrate    - Calibrate LCD");
    Serial.println("  colorbar     - Draw test pattern");
    Serial.println("=======================================");
}

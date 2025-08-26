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
    
    Serial.println("\n=== " PROJECT_NAME " ===");
    Serial.println("Version: " PROJECT_VERSION);
    Serial.println("=====================================\n");
    
    // Initialize system components
    systemReady = systemManager.initializeSystem();
    
    if (systemReady) {
        Serial.println("System ready!");
        printAvailableCommands();
        
        // Display Pokemon image immediately
        Serial.println("Displaying Pokemon image...");
        delay(1000);
        systemManager.displayImage(DEFAULT_IMAGE_PATH);
    } else {
        Serial.println("ERROR: System initialization failed!");
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
    Serial.println("  'show-normal' - Display normal centered image");
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
        Serial.println("Displaying rotated full-screen image...");
        systemManager.displayImage(DEFAULT_IMAGE_PATH);
    }
    else if (command == "show-normal") {
        Serial.println("Displaying normal centered image...");
        systemManager.displayImage(FALLBACK_IMAGE_PATH);
    }
    else if (command == "show-original") {
        Serial.println("Displaying original PNG...");
        systemManager.displayImage(ORIGINAL_PNG_PATH);
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
    Serial.println("Clearing top area...");
    auto lcd = waveshare_lcd_get_instance();
    if (lcd) {
        uint8_t* clearBuffer = (uint8_t*)malloc(1024 * 2 * 50);
        if (clearBuffer) {
            memset(clearBuffer, 0, 1024 * 2 * 50);
            lcd->drawBitmap(0, 0, 1024, 50, clearBuffer);
            free(clearBuffer);
            Serial.println("Top area cleared");
        }
    }
}

void printFullHelp() {
    Serial.println("=== Pokemon Expositor Commands ===");
    Serial.println("IMAGE DISPLAY:");
    Serial.println("  show         - Display rotated full-screen Pokemon");
    Serial.println("  show-normal  - Display normal centered Pokemon");
    Serial.println("  show-original- Display original PNG");
    Serial.println();
    Serial.println("SYSTEM CONTROL:");
    Serial.println("  clear        - Clear screen");
    Serial.println("  clear-top    - Clear top area (fix glitches)");
    Serial.println("  list         - List images on SD card");
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

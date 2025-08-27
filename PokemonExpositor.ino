/*
 * Pokemon Expositor - Main Firmware
 * 
 * Hardware: Waveshare ESP32-S3 RGB LCD + SD Card
 * Features: Pokemon card display with rotation and full-screen support
 */

#include <Arduino.h>
#include "project_config.h"
#include "system_manager.h"
#include "wifi_config_manager.h"

// External global instances
extern WiFiConfigManager wifiConfigManager;

bool systemReady = false;
bool wifiReady = false;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(STARTUP_DELAY_MS);
    
    Serial.println("=== Pokemon Card Expositor ===");
    Serial.println("Inizializzazione sistema...");
    
    // Initialize system components first
    systemReady = systemManager.initializeSystem();
    
    if (systemReady) {
        Serial.println("✅ SystemManager inizializzato");
        
        // Initialize WiFi Config Manager
        wifiReady = wifiConfigManager.initialize();
        
        if (wifiReady) {
            Serial.println("✅ WiFiConfigManager inizializzato");
            
            // Show connection info
            if (wifiConfigManager.isWiFiConnected()) {
                Serial.println("📶 WiFi connesso!");
                Serial.printf("🌐 Interfaccia web: http://%s\n", wifiConfigManager.getLocalIP().c_str());
            } else {
                Serial.println("📶 Modalità Access Point");
                Serial.printf("🌐 Connettiti a 'PokemonExpositor' e vai su http://%s\n", wifiConfigManager.getAPIP().c_str());
            }
        } else {
            Serial.println("⚠️ WiFiConfigManager non inizializzato - solo controllo seriale");
        }
        
        // Display first image from browser
        systemManager.displayCurrentImage();
        
        Serial.println("🚀 Sistema pronto!");
    } else {
        Serial.println("❌ ERRORE: Inizializzazione sistema fallita!");
    }
}

void loop() {
    // Handle web requests if WiFi is ready
    if (wifiReady) {
        wifiConfigManager.handleWebRequests();
    }
    
    // Handle serial commands
    handleSerialCommands();
    
    // Update system manager
    if (systemReady) {
        systemManager.update();
    }
    
    delay(MAIN_LOOP_DELAY_MS);
}

void printAvailableCommands() {
    Serial.println("\n===== AVAILABLE COMMANDS =====");
    
    // Image commands
    Serial.println("📸 Image Commands:");
    Serial.println("  'show' - Display Pokemon image (rotated full-screen)");
    Serial.println("  'next' - Next image");
    Serial.println("  'prev' - Previous image");
    Serial.println("  'current' - Show current image");
    Serial.println("  'clear' - Clear screen");
    Serial.println("  'list' - List images on SD card");
    
    // Brightness commands
    Serial.println("\n💡 Brightness Commands:");
    Serial.println("  'brightness night/low/med/high/max' - Set preset brightness");
    Serial.println("  'brightness <0-255>' - Set custom brightness");
    Serial.println("  'brightness+' / 'brightness-' - Adjust brightness");
    
    #if ENABLE_WIFI_CONFIG
    // WiFi commands  
    Serial.println("\n📶 WiFi Commands:");
    Serial.println("  'wifi ssid <name>' - Set WiFi SSID");
    Serial.println("  'wifi password <pass>' - Set WiFi password");
    Serial.println("  'wifi connect' - Connect to WiFi");
    Serial.println("  'wifi disconnect' - Disconnect from WiFi");
    Serial.println("  'wifi status' - Show WiFi status");
    
    // Slideshow commands
    Serial.println("\n🎬 Slideshow Commands:");
    Serial.println("  'slideshow start' - Start slideshow");
    Serial.println("  'slideshow stop' - Stop slideshow");
    Serial.println("  'slideshow pause' - Pause slideshow");
    Serial.println("  'slideshow next' - Next image");
    Serial.println("  'slideshow prev' - Previous image");
    Serial.println("  'slideshow interval <seconds>' - Set interval");
    Serial.println("  'slideshow brightness <0-255>' - Set brightness");
    Serial.println("  'slideshow random' - Toggle random order");
    Serial.println("  'slideshow loop' - Toggle loop mode");
    Serial.println("  'slideshow status' - Show slideshow status");
    
    // Config commands
    Serial.println("\n⚙️ Config Commands:");
    Serial.println("  'config save' - Save all configurations");
    Serial.println("  'config load' - Load all configurations");
    Serial.println("  'config reset' - Reset to defaults");
    #endif
    
    // System commands
    Serial.println("\n🔧 System Commands:");
    Serial.println("  'test' - Run system tests");
    Serial.println("  'status' - Show system status");
    Serial.println("  'help' - Show all commands");
    Serial.println("===============================\n");
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
    
    #if ENABLE_WIFI_CONFIG
    // WiFi and Slideshow commands
    else if (command == "wifi status") {
        printWiFiStatus();
    }
    else if (command == "wifi ip") {
        if (wifiReady && wifiConfigManager.isWiFiConnected()) {
            Serial.printf("IP: %s\n", wifiConfigManager.getLocalIP().c_str());
        } else {
            Serial.println("WiFi not connected");
        }
    }
    else if (command == "slideshow start") {
        if (wifiReady) {
            Serial.println("Starting slideshow via SystemManager...");
            systemManager.startSlideshow();
        } else {
            Serial.println("WiFi not ready");
        }
    }
    else if (command == "slideshow stop") {
        if (wifiReady) {
            Serial.println("Stopping slideshow...");
            systemManager.stopSlideshow();
        } else {
            Serial.println("WiFi not ready");
        }
    }
    else if (command == "slideshow status") {
        if (wifiReady) {
            Serial.println(wifiConfigManager.getSlideshowStatus());
        } else {
            Serial.println("WiFi not ready");
        }
    }
    else if (command == "wifi debug") {
        printWiFiDebug();
    }
    else if (command == "web test") {
        testWebServer();
    }
    #endif
    
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
    Serial.println();
    #if ENABLE_WIFI_CONFIG
    Serial.println("WIFI & WEB:");
    Serial.println("  wifi status  - Show WiFi connection info");
    Serial.println("  wifi ip      - Show device IP address");
    Serial.println("  wifi debug   - Advanced WiFi diagnostics");
    Serial.println("  web test     - Test web server URLs");
    Serial.println("  slideshow start   - Start slideshow");
    Serial.println("  slideshow stop    - Stop slideshow");
    Serial.println("  slideshow status  - Show slideshow status");
    Serial.println();
    #endif
    Serial.println("=======================================");
}

#if ENABLE_WIFI_CONFIG
void printWiFiStatus() {
    if (!wifiReady) {
        Serial.println("❌ WiFi Config Manager not initialized");
        return;
    }
    
    Serial.println("=== WiFi Status ===");
    Serial.printf("Connected: %s\n", wifiConfigManager.isWiFiConnected() ? "Yes" : "No");
    
    if (wifiConfigManager.isWiFiConnected()) {
        Serial.printf("Local IP: %s\n", wifiConfigManager.getLocalIP().c_str());
        Serial.printf("Web Interface: http://%s\n", wifiConfigManager.getLocalIP().c_str());
    } else {
        Serial.printf("AP IP: %s\n", wifiConfigManager.getAPIP().c_str());
        Serial.printf("Connect to 'PokemonExpositor' and go to: http://%s\n", wifiConfigManager.getAPIP().c_str());
    }
    
    Serial.printf("Web Server: %s\n", wifiConfigManager.isWebServerRunning() ? "Running" : "Stopped");
    Serial.printf("System Ready: %s\n", systemManager.isSystemReady() ? "Yes" : "No");
    Serial.println("==================");
}

void printWiFiDebug() {
    if (!wifiReady) {
        Serial.println("❌ WiFi Config Manager not initialized");
        return;
    }
    
    Serial.println("🔍 === WiFi DEBUG INFO ===");
    Serial.printf("WiFi Manager Ready: %s\n", wifiReady ? "Yes" : "No");
    Serial.printf("WiFi Connected: %s\n", wifiConfigManager.isWiFiConnected() ? "Yes" : "No");
    Serial.printf("Web Server Running: %s\n", wifiConfigManager.isWebServerRunning() ? "Yes" : "No");
    
    if (wifiConfigManager.isWiFiConnected()) {
        Serial.println("📶 STATION MODE (Connected to your WiFi):");
        Serial.printf("  Local IP: %s\n", wifiConfigManager.getLocalIP().c_str());
        Serial.printf("  Access web interface at: http://%s\n", wifiConfigManager.getLocalIP().c_str());
        
        // Test connectivity
        Serial.println("  Testing network connectivity...");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("  WiFi Status: Connected to %s\n", WiFi.SSID().c_str());
            Serial.printf("  Signal Strength: %d dBm\n", WiFi.RSSI());
            Serial.printf("  Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
            Serial.printf("  DNS: %s\n", WiFi.dnsIP().toString().c_str());
        }
    } else {
        Serial.println("📡 ACCESS POINT MODE (Device creates its own WiFi):");
        Serial.printf("  AP IP: %s\n", wifiConfigManager.getAPIP().c_str());
        Serial.println("  Steps to connect:");
        Serial.println("    1. On your phone/PC, go to WiFi settings");
        Serial.println("    2. Look for network 'PokemonExpositor'");
        Serial.println("    3. Connect to it (no password needed)");
        Serial.printf("    4. Open browser and go to: http://%s\n", wifiConfigManager.getAPIP().c_str());
        Serial.println("    5. Configure your WiFi network");
    }
    
    Serial.printf("System Status: %s\n", systemManager.isSystemReady() ? "Ready" : "Not Ready");
    Serial.println("========================");
}

void testWebServer() {
    if (!wifiReady) {
        Serial.println("❌ WiFi not ready - cannot test web server");
        return;
    }
    
    Serial.println("🧪 === WEB SERVER TEST ===");
    Serial.printf("WiFi Manager Ready: %s\n", wifiReady ? "✅ Yes" : "❌ No");
    Serial.printf("Web Server Status: %s\n", wifiConfigManager.isWebServerRunning() ? "✅ Running" : "❌ Not Running");
    
    // Test basic connectivity
    if (wifiConfigManager.isWiFiConnected()) {
        String ip = wifiConfigManager.getLocalIP();
        Serial.printf("📡 STATION MODE - Connected to WiFi\n");
        Serial.printf("🌐 Device IP: %s\n", ip.c_str());
        Serial.printf("🔗 Test URLs:\n");
        Serial.printf("   Main page: http://%s/\n", ip.c_str());
        Serial.printf("   Config: http://%s/config\n", ip.c_str());
        Serial.printf("   Status: http://%s/system-status\n", ip.c_str());
        Serial.printf("   Logs: http://%s/get-logs\n", ip.c_str());
    } else {
        Serial.printf("📡 ACCESS POINT MODE\n");
        String apIP = wifiConfigManager.getAPIP();
        Serial.printf("🌐 AP IP: %s\n", apIP.c_str());
        Serial.printf("🔗 Test URLs:\n");
        Serial.printf("   Main page: http://%s/\n", apIP.c_str());
        Serial.printf("   Config: http://%s/config\n", apIP.c_str());
        Serial.printf("   Status: http://%s/system-status\n", apIP.c_str());
    }
    
    Serial.println("💡 Troubleshooting:");
    Serial.println("   1. Make sure you're connected to the same network");
    Serial.println("   2. Use http:// not https://");
    Serial.println("   3. Try different browsers (Chrome recommended)");
    Serial.println("   4. Disable VPN if active");
    Serial.println("   5. Check firewall settings");
    
    // Test web server functionality
    Serial.println("\n🔧 Testing web server response...");
    Serial.println("   (Check if server responds to handleClient() calls)");
    for (int i = 0; i < 5; i++) {
        wifiConfigManager.handleWebRequests();
        delay(100);
    }
    Serial.println("   ✅ handleWebRequests() calls completed");
    
    Serial.println("==========================");
}
#endif

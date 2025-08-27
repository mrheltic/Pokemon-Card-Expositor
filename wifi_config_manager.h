#ifndef __WIFI_CONFIG_MANAGER_H
#define __WIFI_CONFIG_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <FS.h>
#include "project_config.h"

// WiFi Configuration Structure
struct WiFiConfig {
    String ssid;
    String password;
    String deviceName;
    String hostname;
    bool enableAP;
    String apPassword;
};

// Slideshow Configuration Structure
struct SlideshowConfig {
    bool enabled;
    uint32_t intervalMs;
    bool randomOrder;
    bool loop;
    uint8_t brightness;
    String imageFilter;        // File extension filter (e.g., ".raw")
    bool autoStart;
    bool disableBrightness;    // when true, keep display off for slideshow
    String scalingMode;        // fit, fill, center, stretch
    uint16_t rotationAngle;    // 0, 90, 180, 270
};

// Advanced Configuration Structure
struct AdvancedConfig {
    bool enablePreloading;
    uint8_t cacheSize;
    uint32_t maxFileSize;      // in bytes
    bool enableDeepSleep;
    uint32_t sleepTimeout;     // in ms
    String logLevel;           // DEBUG, INFO, WARN, ERROR
    bool enableSerial;
    bool enableDebug;
    bool enableErrorRecovery;
    uint32_t watchdogTimeout;  // in ms
};

// Display Configuration Structure
struct DisplayConfig {
    uint8_t brightness;        // 0-255
    uint8_t contrast;          // 0-255
    String scalingMode;        // fit, fill, center, stretch
    uint16_t rotation;         // 0, 90, 180, 270
    bool flipHorizontal;
    bool flipVertical;
    String colorProfile;       // default, vivid, warm, cool
    bool enableDithering;
    bool enableAntiAliasing;
    uint32_t screensaverTimeout; // in seconds, 0 = disabled
};

// System Status Structure
struct SystemStatus {
    bool wifiConnected;
    String localIP;
    String apIP;
    uint32_t uptime;
    uint32_t freeHeap;
    String chipModel;
    uint16_t imagesCount;
    uint32_t totalSpace;
    uint32_t usedSpace;
    uint32_t freeSpace;
};

class WiFiConfigManager {
private:
    WebServer* webServer;
    WiFiConfig wifiConfig;
    SlideshowConfig slideshowConfig;
    AdvancedConfig advancedConfig;
    DisplayConfig displayConfig;
    bool wifiConnected;
    bool webServerStarted;
    bool configLoaded;
    
    // File paths
    static const char* WIFI_CONFIG_FILE;
    static const char* SLIDESHOW_CONFIG_FILE;
    static const char* ADVANCED_CONFIG_FILE;
    static const char* DISPLAY_CONFIG_FILE;
    
    // Web server handlers
    void handleRoot();
    void handleConfig();
    void handleSaveConfig();
    void handleGetConfig();
    void handleSlideshowControl();
    void handleRestart();
    void handleNotFound();
    void handleAdvancedConfig();
    void handleSaveAdvanced();
    void handleDisplayConfig();
    void handleSaveDisplay();
    void handleGetLogs();
    void handleClearLogs();
    void handleGetSystemStatus();
    void handleStorageOperation();
    void handleBrightnessControl();
    void handleApplySettings();
    
    // Configuration management
    bool loadWiFiConfig();
    bool loadSlideshowConfig();
    bool loadAdvancedConfig();
    bool loadDisplayConfig();
    bool saveWiFiConfig();
    bool saveSlideshowConfig();
    bool saveAdvancedConfig();
    bool saveDisplayConfig();
    void setDefaultConfigs();
    
    // System management
    SystemStatus getSystemStatus();
    String getSystemLogs();
    void clearSystemLogs();
    void addLogEntry(const String& level, const String& message);
    
    // Storage management
    bool formatStorage();
    bool cleanupTempFiles();
    uint32_t getImageCount();
    
    // WiFi management
    bool connectToWiFi();
    void startAccessPoint();
    void printNetworkInfo();
    
public:
    WiFiConfigManager();
    ~WiFiConfigManager();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // WiFi control
    bool isWiFiConnected() const { return wifiConnected; }
    String getLocalIP() const;
    String getAPIP() const;
    
    // Web server control
    void handleWebRequests();
    bool isWebServerRunning() const { return webServerStarted; }
    
    // Configuration access
    const WiFiConfig& getWiFiConfig() const { return wifiConfig; }
    const SlideshowConfig& getSlideshowConfig() const { return slideshowConfig; }
    const AdvancedConfig& getAdvancedConfig() const { return advancedConfig; }
    const DisplayConfig& getDisplayConfig() const { return displayConfig; }
    
    // Configuration updates (for serial commands)
    bool updateWiFiConfig(const WiFiConfig& newConfig);
    bool updateSlideshowConfig(const SlideshowConfig& newConfig);
    bool updateAdvancedConfig(const AdvancedConfig& newConfig);
    bool updateDisplayConfig(const DisplayConfig& newConfig);
    
    // Serial command interface
    void handleSerialCommand(const String& command, const String& params);
    void printStatus();
    void printHelp();
    
    // Slideshow control
    bool startSlideshow();
    bool stopSlideshow();
    bool pauseSlideshow();
    bool nextImage();
    bool previousImage();
    
    // System integration
    void applyBrightnessSettings();
    void applyDisplaySettings();
    bool isSystemReady();
    
    // Status queries
    bool isSlideshowEnabled() const { return slideshowConfig.enabled; }
    uint32_t getSlideshowInterval() const { return slideshowConfig.intervalMs; }
    uint8_t getSlideshowBrightness() const { return slideshowConfig.brightness; }
    String getSlideshowStatus();
};

#endif

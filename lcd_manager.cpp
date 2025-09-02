#include "lcd_manager.h"
#include "sd_manager.h"

LCDManager lcdManager;
static esp_panel::drivers::LCD* displayInstance = nullptr;

LCDManager::LCDManager() : initialized(false) {
}

LCDManager::~LCDManager() {
    deinit();
}

bool LCDManager::init() {
    if (initialized) {
        return true;
    }

    Serial.println("Initializing LCD Manager...");
    
    // Make sure SD manager is initialized first (for IO expander)
    if (!sdManager.isInitialized()) {
        Serial.println("SD Manager must be initialized first for IO expander access");
        return false;
    }
    
    // Initialize the RGB LCD
    waveshare_lcd_init();
    
    // Get LCD instance
    displayInstance = waveshare_lcd_get_instance();
    if (!displayInstance) {
        Serial.println("Failed to get LCD instance");
        return false;
    }
    
    // Turn on backlight through IO expander
    auto expander = sdManager.getExpander();
    if (expander) {
        expander->digitalWrite(LCD_BL, HIGH);
    }
    
    // Fix white border issue by clearing screen
    // delay(500);
    // fillScreen(0x0000); // Fill with black to remove any border
    
    initialized = true;
    Serial.println("LCD Manager initialized successfully - skipping border fix");
    return true;
}

void LCDManager::deinit() {
    if (initialized) {
        // Turn off backlight
        auto expander = sdManager.getExpander();
        if (expander) {
            expander->digitalWrite(LCD_BL, LOW);
        }
        
        initialized = false;
        Serial.println("LCD Manager deinitialized");
    }
}

void LCDManager::setBacklight(bool state) {
    if (!initialized) {
        Serial.println("LCD Manager not initialized");
        return;
    }
    
    auto expander = sdManager.getExpander();
    if (expander) {
        expander->digitalWrite(LCD_BL, state ? HIGH : LOW);
        Serial.printf("LCD backlight %s\n", state ? "ON" : "OFF");
    }
}

void LCDManager::clearScreen() {
    if (!initialized || !displayInstance) {
        Serial.println("LCD Manager not initialized");
        return;
    }
    
    // Fill the entire screen with black color (0x0000)
    fillScreen(0x0000);
    Serial.println("Screen cleared (filled with black)");
}

void LCDManager::fillScreen(uint16_t color) {
    if (!initialized || !displayInstance) {
        Serial.println("LCD Manager not initialized");
        return;
    }
    
    // Get screen dimensions with safety checks
    int width = EXAMPLE_LCD_WIDTH;
    int height = EXAMPLE_LCD_HEIGHT;
    
    // Validate dimensions
    if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
        Serial.printf("ERROR: Invalid screen dimensions: %dx%d\n", width, height);
        return;
    }
    
    // Create a buffer for one row (width * 2 bytes per pixel for 16-bit color)
    size_t row_size = (size_t)width * 2; // 2 bytes per pixel for 16-bit color
    
    // Check for potential overflow
    if (row_size < width || row_size > SIZE_MAX / 2) {
        Serial.println("ERROR: Row size calculation overflow");
        return;
    }
    
    uint8_t* rowBuffer = (uint8_t*)malloc(row_size);
    
    if (!rowBuffer) {
        Serial.printf("Failed to allocate %zu bytes for screen fill\n", row_size);
        return;
    }
    
    // Fill buffer with the desired color (convert 16-bit to bytes)
    uint16_t* colorPtr = (uint16_t*)rowBuffer;
    for (int i = 0; i < width; i++) {
        colorPtr[i] = color;
    }
    
    // Fill screen row by row
    for (int y = 0; y < height; y++) {
        displayInstance->drawBitmap(0, y, width, 1, rowBuffer, 100);
        
        // Yield occasionally to prevent watchdog timeout
        if (y % 50 == 0) {
            yield();
        }
    }
    
    free(rowBuffer);
    Serial.printf("Screen filled with color: 0x%04X\n", color);
}

void LCDManager::calibrateDisplay() {
    if (!initialized || !displayInstance) {
        Serial.println("LCD Manager not initialized");
        return;
    }
    
    Serial.println("Calibrating display to fix white border...");
    
    // First, fill with black to remove any white borders
    fillScreen(0x0000);
    delay(500);
    
    // Test with different colors to verify full screen coverage
    uint16_t testColors[] = {
        0x0000, // Black
        0xF800, // Red  
        0x07E0, // Green
        0x001F, // Blue
        0xFFFF  // White
    };
    
    for (int i = 0; i < 5; i++) {
        fillScreen(testColors[i]);
        delay(1000);
    }
    
    // End with black screen
    fillScreen(0x0000);
    Serial.println("Display calibration completed");
}

void LCDManager::displayMessage(const char* message) {
    if (!initialized) {
        Serial.println("LCD Manager not initialized");
        return;
    }
    
    Serial.printf("Displaying message: %s\n", message);
    // This would need to be implemented based on the actual LCD library functions
}

void LCDManager::drawColorBar() {
    if (!initialized || !displayInstance) {
        Serial.println("LCD Manager not initialized");
        return;
    }
    
    Serial.println("Drawing color bar without white border...");
    
    // First clear any potential border
    fillScreen(0x0000);
    delay(200);
    
    // Now draw the color bar test
    displayInstance->colorBarTest();
}

void LCDManager::runLCDTest() {
    if (!initialized) {
        Serial.println("LCD Manager not initialized - attempting to initialize...");
        if (!init()) {
            Serial.println("Failed to initialize LCD Manager");
            return;
        }
    }
    
    Serial.println("=== LCD Test Started ===");
    Serial.println("RGB LCD example start");
    
    // Turn on backlight
    setBacklight(true);
    
    // Calibrate display to fix white border
    calibrateDisplay();
    
    // Draw color bar test
    drawColorBar();
    
    Serial.println("RGB LCD example end");
    Serial.println("=== LCD Test Completed ===");
}

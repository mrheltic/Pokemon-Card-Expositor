#ifndef __PROJECT_CONFIG_H
#define __PROJECT_CONFIG_H

// Project Information
#define PROJECT_NAME "Pokemon Expositor"
#define PROJECT_VERSION "1.0.0"
#define PROJECT_AUTHOR "Pokemon Card Expositor Project"

// Debug Configuration
#define DEBUG_ENABLED 1
#define DEBUG_SD_OPERATIONS 1
#define DEBUG_LCD_OPERATIONS 1
// DMA logging: set to 1 to enable verbose DMA logs, 0 to disable
#define DMA_VERBOSE_LOGGING 0

// System Configuration
#define SYSTEM_STATUS_INTERVAL_MS 10000
#define SERIAL_BAUD_RATE 115200
#define STARTUP_DELAY_MS 2000

// Feature Flags
#define ENABLE_SD_CARD 1
#define ENABLE_LCD_DISPLAY 1
#define ENABLE_SERIAL_COMMANDS 1
#define ENABLE_AUTO_TESTS 1
#define ENABLE_IMAGE_DISPLAY 1
#define ENABLE_WIFI_CONFIG 1
#define ENABLE_WEB_INTERFACE 1
#define ENABLE_SLIDESHOW 1

// WiFi Configuration
#define WIFI_CONNECT_TIMEOUT_MS 10000
#define WIFI_AP_DEFAULT_PASSWORD "pokemon123"
#define WEB_SERVER_PORT 80
#define CONFIG_JSON_SIZE 2048

// Slideshow Configuration
#define SLIDESHOW_DEFAULT_INTERVAL_MS 1000
#define SLIDESHOW_MIN_INTERVAL_MS 1000
#define SLIDESHOW_MAX_INTERVAL_MS 3600000
#define SLIDESHOW_DEFAULT_BRIGHTNESS 0
#define SLIDESHOW_TRANSITION_MS 500

// Image Processing Configuration
// #define USE_PNGDEC_LIBRARY 1  // Uncomment if PNGdec library is available
#define ENABLE_TEST_PATTERNS 1
#define DEFAULT_IMAGE_PATH "/pokemon_rotated.raw"
#define FALLBACK_IMAGE_PATH "/pokemon.raw"
#define ORIGINAL_PNG_PATH "/131_hires_1.png"

// LCD Specific Configuration
#define LCD_BORDER_FIX_ENABLED 1
#define LCD_REDUCED_BOUNCE_BUFFER 1
#define LCD_BORDER_FIX_DELAY_MS 500

// Display dimensions
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 600

// Timing Configuration
#define MAIN_LOOP_DELAY_MS 100
#define COMMAND_TIMEOUT_MS 5000

#endif

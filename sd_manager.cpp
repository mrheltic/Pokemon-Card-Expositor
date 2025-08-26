#include "sd_manager.h"

SDManager sdManager;

SDManager::SDManager() : expander(nullptr), initialized(false) {
}

SDManager::~SDManager() {
    deinit();
}

bool SDManager::init() {
    if (initialized) {
        return true;
    }

    Serial.println("Initialize IO expander for SD");
    
    // Initialize IO expander
    expander = new esp_expander::CH422G(EXAMPLE_I2C_SCL_PIN, EXAMPLE_I2C_SDA_PIN, EXAMPLE_I2C_ADDR);
    if (!expander) {
        Serial.println("Failed to create IO expander");
        return false;
    }
    
    expander->init();
    expander->begin();
    expander->multiPinMode(TP_RST | LCD_BL | LCD_RST | SD_CS | USB_SEL, OUTPUT);
    expander->multiDigitalWrite(TP_RST | LCD_BL | LCD_RST, HIGH);

    // Use extended GPIO for SD card
    expander->digitalWrite(SD_CS, LOW);

    // Turn off backlight initially
    expander->digitalWrite(LCD_BL, LOW);

    // When USB_SEL is HIGH, it enables FSUSB42UMX chip and gpio19, gpio20 wired CAN_TX CAN_RX
    expander->digitalWrite(USB_SEL, LOW);

    // Initialize SPI
    SPI.setHwCs(false);
    SPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
    
    if (!SD.begin(SD_SS)) {
        Serial.println("SD Card Mount Failed");
        delete expander;
        expander = nullptr;
        return false;
    }

    initialized = true;
    Serial.println("SD Manager initialized successfully");
    return true;
}

void SDManager::deinit() {
    if (initialized) {
        SD.end();
        if (expander) {
            delete expander;
            expander = nullptr;
        }
        initialized = false;
        Serial.println("SD Manager deinitialized");
    }
}

void SDManager::printCardInfo() {
    if (!initialized) {
        Serial.println("SD Manager not initialized");
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void SDManager::runFileSystemTest() {
    if (!initialized) {
        Serial.println("SD Manager not initialized");
        return;
    }

    Serial.println("Running file system test...");
    
    // Testing file system functionality
    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    
    Serial.println("File system test completed");
}

void SDManager::runSDTest() {
    if (!initialized) {
        Serial.println("SD Manager not initialized - attempting to initialize...");
        if (!init()) {
            Serial.println("Failed to initialize SD Manager");
            return;
        }
    }
    
    Serial.println("=== SD Card Test Started ===");
    printCardInfo();
    runFileSystemTest();
    Serial.println("=== SD Card Test Completed ===");
}

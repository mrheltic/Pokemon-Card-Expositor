#include "image_browser.h"
#include "project_config.h"

ImageBrowser imageBrowser;

ImageBrowser::ImageBrowser() : currentIndex(0), initialized(false) {
}

ImageBrowser::~ImageBrowser() {
    deinit();
}

bool ImageBrowser::init() {
    if (initialized) {
        Serial.println("[ImageBrowser] Already initialized");
        return true;
    }
    
    Serial.println("[ImageBrowser] Initializing...");
    
    // SD card is already initialized by SD Manager, just check if it's available
    if (!SD.exists("/")) {
        Serial.println("[ImageBrowser] ERROR: SD card not available");
        return false;
    }
    
    Serial.println("[ImageBrowser] SD card OK, scanning for images...");
    scanSDCard();
    
    if (hasImages()) {
        Serial.printf("[ImageBrowser] ✅ Successfully initialized with %d images\n", getImageCount());
        initialized = true;
        return true;
    } else {
        Serial.println("[ImageBrowser] ❌ No images found - initialization failed");
        return false;
    }
}

void ImageBrowser::deinit() {
    imageList.clear();
    currentIndex = 0;
    initialized = false;
}

bool ImageBrowser::isImageFile(const String& filename) {
    String lower = filename;
    lower.toLowerCase();
    
    return lower.endsWith(".png") || 
           lower.endsWith(".jpg") || 
           lower.endsWith(".jpeg") || 
           lower.endsWith(".bmp") || 
           lower.endsWith(".raw");
}

void ImageBrowser::scanSDCard() {
    imageList.clear();
    
    // First try to scan /images/ directory
    File imagesDir = SD.open("/images");
    File root;
    
    if (imagesDir && imagesDir.isDirectory()) {
        Serial.println("[ImageBrowser] Scanning /images/ directory");
        root = imagesDir;
    } else {
        Serial.println("[ImageBrowser] /images/ directory not found, scanning root directory");
        root = SD.open("/");
        if (!root) {
            Serial.println("[ImageBrowser] ERROR: Cannot open SD card root");
            return;
        }
    }
    
    File file = root.openNextFile();
    int imageCount = 0;
    int maxFiles = 5000; // Safety limit for file iteration
    int fileCounter = 0;
    
    while (file && fileCounter < maxFiles) {
        if (!file.isDirectory()) {
            String filename = file.name();
            if (isImageFile(filename)) {
                // If scanning /images/, add full path
                if (imagesDir && imagesDir.isDirectory()) {
                    imageList.push_back("/images/" + filename);
                } else {
                    imageList.push_back("/" + filename);
                }
                imageCount++;
                Serial.printf("[ImageBrowser] Found image: %s\n", filename.c_str());
            }
        }
        file.close(); // Explicitly close file
        file = root.openNextFile();
        fileCounter++;
    }
    
    if (fileCounter >= maxFiles) {
        Serial.printf("[ImageBrowser] WARNING: Reached maximum file limit (%d), scan may be incomplete\n", maxFiles);
    }
    
    root.close();
    
    Serial.printf("[ImageBrowser] Total images found: %d\n", imageCount);
    
    // Sort the list for consistent order
    std::sort(imageList.begin(), imageList.end());
    
    // Set current index to first image or 0 if none
    currentIndex = hasImages() ? 0 : -1;
}

bool ImageBrowser::nextImage() {
    if (!hasImages()) {
        return false;
    }
    
    currentIndex = (currentIndex + 1) % imageList.size();
    return true;
}

bool ImageBrowser::previousImage() {
    if (!hasImages()) {
        return false;
    }
    
    currentIndex = (currentIndex - 1 + imageList.size()) % imageList.size();
    return true;
}

bool ImageBrowser::goToFirst() {
    if (!hasImages()) {
        return false;
    }
    
    currentIndex = 0;
    return true;
}

bool ImageBrowser::goToLast() {
    if (!hasImages()) {
        return false;
    }
    
    currentIndex = imageList.size() - 1;
    return true;
}

bool ImageBrowser::goToIndex(int index) {
    if (!hasImages() || index < 0 || index >= (int)imageList.size()) {
        return false;
    }
    
    currentIndex = index;
    return true;
}

String ImageBrowser::getCurrentImagePath() {
    if (!hasImages() || currentIndex < 0 || currentIndex >= (int)imageList.size()) {
        Serial.printf("[ImageBrowser] WARNING: Invalid state - hasImages:%s, currentIndex:%d, size:%d\n", 
                     hasImages() ? "true" : "false", currentIndex, (int)imageList.size());
        return "";
    }
    
    return imageList[currentIndex];
}

String ImageBrowser::getCurrentImageName() {
    String path = getCurrentImagePath();
    if (path.length() == 0) {
        return "";
    }
    
    int lastSlash = path.lastIndexOf('/');
    if (lastSlash >= 0) {
        return path.substring(lastSlash + 1);
    }
    
    return path;
}

void ImageBrowser::refreshImageList() {
    scanSDCard();
}

void ImageBrowser::printImageList() {
    if (!hasImages()) {
        Serial.println("No images found on SD card");
        return;
    }
    
    Serial.printf("Found %d images:\n", imageList.size());
    for (int i = 0; i < (int)imageList.size(); i++) {
        String marker = (i == currentIndex) ? " > " : "   ";
        Serial.printf("%s%d: %s\n", marker.c_str(), i + 1, imageList[i].c_str());
    }
}

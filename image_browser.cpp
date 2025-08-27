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
        return true;
    }
    
    if (!SD.begin()) {
        return false;
    }
    
    scanSDCard();
    initialized = true;
    
    return hasImages();
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
    
    File root = SD.open("/");
    if (!root) {
        return;
    }
    
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filename = file.name();
            if (isImageFile(filename)) {
                imageList.push_back("/" + filename);
            }
        }
        file = root.openNextFile();
    }
    
    root.close();
    
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
    if (!hasImages() || currentIndex < 0) {
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

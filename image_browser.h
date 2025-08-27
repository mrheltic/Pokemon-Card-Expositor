#ifndef IMAGE_BROWSER_H
#define IMAGE_BROWSER_H

#include <Arduino.h>
#include <vector>
#include "SD.h"

class ImageBrowser {
private:
    std::vector<String> imageList;
    int currentIndex;
    bool initialized;
    
    // Supported image formats
    bool isImageFile(const String& filename);
    void scanSDCard();
    
public:
    ImageBrowser();
    ~ImageBrowser();
    
    bool init();
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Navigation
    bool nextImage();
    bool previousImage();
    bool goToFirst();
    bool goToLast();
    bool goToIndex(int index);
    
    // Current image info
    String getCurrentImagePath();
    String getCurrentImageName();
    int getCurrentIndex() const { return currentIndex; }
    int getImageCount() const { return imageList.size(); }
    
    // List management
    void refreshImageList();
    void printImageList();
    
    // Validation
    bool hasImages() const { return !imageList.empty(); }
};

extern ImageBrowser imageBrowser;

#endif // IMAGE_BROWSER_H

#ifndef __PNG_DECODER_H
#define __PNG_DECODER_H

#include <Arduino.h>
#include "FS.h"
#include "SD.h"

// Struttura per header PNG
struct PNGHeader {
    uint32_t width;
    uint32_t height;
    uint8_t bitDepth;
    uint8_t colorType;
    uint8_t compressionMethod;
    uint8_t filterMethod;
    uint8_t interlaceMethod;
};

// Decoder PNG semplificato
class SimplePNGDecoder {
private:
    File file;
    PNGHeader header;
    bool headerParsed;
    
    bool parseHeader();
    bool readChunk(uint32_t* length, char* type, uint8_t** data);
    uint32_t crc32(uint8_t* data, size_t length);
    
public:
    SimplePNGDecoder();
    ~SimplePNGDecoder();
    
    bool open(const char* filepath);
    void close();
    
    bool getHeader(PNGHeader* outHeader);
    bool decodeToRGB565(uint16_t** outData, uint32_t* outSize);
    
    // Utility functions
    static void convertRGBtoRGB565(uint8_t r, uint8_t g, uint8_t b, uint16_t* rgb565);
    static bool isValidPNG(const char* filepath);
};

#endif

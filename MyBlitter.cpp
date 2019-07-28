
#include "MyBlitter.h"

MyBlitter::MyBlitter() {}

void MyBlitter::blitShader(float x0, int y, float x1, GPixel* row, const GPaint& paint, const GBitmap& fDevice) {

    int start_x = GRoundToInt(x0);
    int curr_x = GRoundToInt(x0);
    int end_x = GRoundToInt(x1);

    if (start_x == end_x) {return;}

    if (curr_x >= end_x) {return;}
    int i = (y * fDevice.width()) + curr_x;
    unsigned dA, dR, dG, dB;
    unsigned sA, sR, sG, sB;
    while (curr_x < end_x) {
        if (fDevice.width() * fDevice.height() < i || i < 0){
            // for debugging purposes! "a point has been detected as out of bounds"
            analyze(fDevice.width(), fDevice.height(), y, curr_x, end_x, fDevice);
        }
        sA = GPixel_GetA(row[curr_x - start_x]); // V TODO: replace division
        sR = GPixel_GetR(row[curr_x - start_x]); if (sR > sA) {sR = sR * sA / 255;}
        sG = GPixel_GetG(row[curr_x - start_x]); if (sG > sA) {sG = sG * sA / 255;}
        sB = GPixel_GetB(row[curr_x - start_x]); if (sB > sA) {sB = sB * sA / 255;}
        dA = GPixel_GetA(fDevice.pixels()[i]);
        dR = GPixel_GetR(fDevice.pixels()[i]);
        dG = GPixel_GetG(fDevice.pixels()[i]);
        dB = GPixel_GetB(fDevice.pixels()[i]);
        fDevice.pixels()[i] 
                = GPixel_PackARGB(getByte(sA, dA, sA, dA, paint.getBlendMode()), 
                getByte(sR, dR, sA, dA, paint.getBlendMode()), 
                getByte(sG, dG, sA, dA, paint.getBlendMode()), 
                getByte(sB, dB, sA, dA, paint.getBlendMode()));
        curr_x++;
        i++;
    }
}

void MyBlitter::blitColor(float x0, int y, float x1, const GPaint& paint, const GBitmap& fDevice) {
    int curr_x = GRoundToInt(x0);
    int end_x = GRoundToInt(x1);

    if (curr_x == end_x) {return;}

    if (curr_x < 0) {curr_x = 0;} if (end_x >= fDevice.width()) {end_x = fDevice.width() - 1;}
    if (curr_x >= end_x || end_x <= 0 || curr_x >= fDevice.width()) {return;}
    int i = (y * fDevice.width()) + curr_x;
    unsigned dA, dR, dG, dB;
    unsigned sA = GRoundToInt(paint.getAlpha() * 255);
    if (paint.getAlpha() < 0) {return;}
    unsigned sR = GRoundToInt((paint.getAlpha() * paint.getColor().fR) * 255);
    unsigned sG = GRoundToInt((paint.getAlpha() * paint.getColor().fG) * 255);
    unsigned sB = GRoundToInt((paint.getAlpha() * paint.getColor().fB) * 255);
    if (sA > 255) {sA = (unsigned)((int)sA * -1);}
    if (sR > 255) {sR = (unsigned)((int)sR * -1);} // TODO: are these interpreting the error correctly?
    if (sG > 255) {sG = (unsigned)((int)sG * -1);}
    if (sB > 255) {sB = (unsigned)((int)sB * -1);}
    while (curr_x < end_x) {
        if (fDevice.width() * fDevice.height() < i || i < 0){
            //a point has been detected as out of bounds
            analyze(fDevice.width(), fDevice.height(), y, curr_x, end_x, fDevice);
        }
        dA = GPixel_GetA(fDevice.pixels()[i]);
        dR = GPixel_GetR(fDevice.pixels()[i]);
        dG = GPixel_GetG(fDevice.pixels()[i]);
        dB = GPixel_GetB(fDevice.pixels()[i]);

        fDevice.pixels()[i] 
                = GPixel_PackARGB(getByte(sA, dA, sA, dA, paint.getBlendMode()), 
                getByte(sR, dR, sA, dA, paint.getBlendMode()), 
                getByte(sG, dG, sA, dA, paint.getBlendMode()), 
                getByte(sB, dB, sA, dA, paint.getBlendMode()));
        curr_x++;
        i++;
    }
}

void MyBlitter::testBlitColor(float x0, int y, float x1, const GPaint& paint, const GBitmap& fDevice) {
    int curr_x = GRoundToInt(x0);
    int end_x = GRoundToInt(x1);

    if (curr_x == end_x) {return;}

    if (curr_x < 0) {curr_x = 0;} if (end_x >= fDevice.width()) {end_x = fDevice.width() - 1;}
    if (curr_x >= end_x || end_x <= 0 || curr_x >= fDevice.width()) {return;}
    int i = (y * fDevice.width()) + curr_x;
    unsigned sA = 254;
    unsigned sR = 254;
    unsigned sG = 0;
    unsigned sB = 0;
    while (curr_x < end_x) {
        fDevice.pixels()[i] 
                = GPixel_PackARGB(sA, sR, sG, sB);
        curr_x++;
        i++;
    }
}
    
unsigned MyBlitter::getByte(unsigned S, unsigned D, unsigned Sa, unsigned Da, GBlendMode blendMode){
    unsigned returnVal = 0;
    switch(blendMode){
        case GBlendMode::kClear:
            returnVal = 0;
            if (returnVal > 255) {printf("kClear %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kSrc:
            returnVal = S;
            if (returnVal > 255) {printf("kSrc %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kDst:
            returnVal = D;
            if (returnVal > 255) {printf("kDst %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kSrcOver:
            returnVal = S + (255 - Sa) * D / 255;
            if (returnVal > 255) {printf("kSrcOver %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kDstOver:
            returnVal = D + (255 - Da) * S / 255;
            if (returnVal > 255) {printf("kDstOver %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kSrcIn:
            returnVal = Da * S / 255;
            if (returnVal > 255) {printf("kSrcIn %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kDstIn:
            returnVal = Sa * D / 255;
            if (returnVal > 255) {printf("kDstIn %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kSrcOut:
            returnVal = (255 - Da)*S / 255;
            if (returnVal > 255) {printf("kSrcOut %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kDstOut:
            returnVal = (255 - Sa)*D / 255;
            if (returnVal > 255) {printf("kDstOut %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kSrcATop:
            returnVal = (Da*S + (255 - Sa)*D) / 255;
            if (returnVal > 255) {printf("kSrcATop %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kDstATop:
            returnVal = (Sa*D + (255 - Da)*S) / 255;
            if (returnVal > 255) {printf("kDstATop %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
        case GBlendMode::kXor:
            returnVal = ((255 - Sa)*D + (255 - Da)*S)/255;
            if (returnVal > 255) {printf("kXor %u %u %u %u %u \n", S, D, Sa, Da, returnVal);}
            return returnVal;
    }
    return 0;
}
    
// used for debugging
void MyBlitter::analyze(int width, int height, int y_0, float x_0, float x_1, const GBitmap& fDevice){
    int i = (y_0 * fDevice.width()) + x_0;
    unsigned dA, dR, dG, dB;
    dA = GPixel_GetA(fDevice.pixels()[i]);
    dR = GPixel_GetR(fDevice.pixels()[i]);
    dG = GPixel_GetG(fDevice.pixels()[i]);
    dB = GPixel_GetB(fDevice.pixels()[i]);
}


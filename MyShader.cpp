
#include <stdio.h>
#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "include/GPoint.h"
#include "include/GMath.h"

class MyShader : public GShader {
public:
    ~MyShader(){}
    
    MyShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode mode) {
        shaderMode = mode;
        shaderBitmap = bitmap;
        shaderLocalMatrix = localMatrix;
    }
    
    MyShader(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode mode) {
        shaderMode = mode;
        gColors = (GColor*) malloc(count * sizeof(GColor));
        memcpy(gColors, colors, count * sizeof(GColor));
        gcCount = count;
        
        float dx = p1.x() - p0.x();
        float dy = p1.y() - p0.y();
        shaderLocalMatrix.set6(
            dx, -dy, p0.x(),
            dy, dx, p0.y());
        // no bitmap
    }
    
    bool isOpaque() override {
        return 0;
        // <- TODO: right now it just assumes some pixel won't be opaque. fix this for efficiency(?)
    }

    bool setContext(const GMatrix& ctm) override {
        GMatrix temp;
        temp.setConcat(ctm, shaderLocalMatrix);
        return temp.invert(&fInverse);
        
    }

    void shadeRow(int x, int y, int count, GPixel result[]) override {
        float c, u, v; // c = local.fX; u = proportion of c0; v = proportion of c1
        int index; // index of c0
        GColor c0, c1; // colors being used to find gradient color
        GPoint local; // point of interest
        if (shaderBitmap.width() == 0) { // use gradient
            
            if (shaderMode == GShader::TileMode::kClamp){
                for (int i = 0; i < count; ++i) {
                    local = fInverse.mapXY(x + i, y);
                    c = local.fX;
                    if (c <= 0.0f || gcCount == 1) {
                        result[i] = getPixel(gColors[0].fA, gColors[0].fR, gColors[0].fG, gColors[0].fB);
                    }
                    else if (c >= 1.0f) {result[i] = getPixel(gColors[gcCount - 1].fA, gColors[gcCount - 1].fR, gColors[gcCount - 1].fG, gColors[gcCount - 1].fB);}
                    else {
                        index = GFloorToInt(c * (gcCount - 1));
                        int rangeCount = gcCount - 1;
                        float rangeLength = 1.0f / (gcCount - 1);
                        while (c - rangeLength >= 0.0f){
                            c -= rangeLength;
                        }
                        u = c * rangeCount;
                        v = 1.0f - u;
                        c1 = gColors[index];
                        c0 = gColors[index + 1];
                        result[i] = getPixel(
                                (c0.fA * u) + (c1.fA * v),
                                (c0.fR * u) + (c1.fR * v),
                                (c0.fG * u) + (c1.fG * v),
                                (c0.fB * u) + (c1.fB * v));
                    }
                }
            } else if (shaderMode == GShader::TileMode::kMirror){
                for (int i = 0; i < count; ++i) {
                    local = fInverse.mapXY(x + i, y);
                    c = local.fX;
                    if (c < 0.0f) {
                        if ((GFloorToInt(c) % 2) == 0) {c = 1.0f - (c - GFloorToInt(c));}
                        else {c = c - GFloorToInt(c);}
                    } else {
                        if ((GFloorToInt(c) % 2) == 0) {c = c - GFloorToInt(c);}
                        else {c = 1.0f - (c - GFloorToInt(c));}
                    }
                    index = GFloorToInt(c * (gcCount - 1));
                    int rangeCount = gcCount - 1;
                    float rangeLength = 1.0f / (gcCount - 1);
                    while (c - rangeLength >= 0.0f){
                        c -= rangeLength;
                    }
                    u = c * rangeCount;
                    v = 1.0f - u;
                    c1 = gColors[index];
                    c0 = gColors[index + 1];
                    result[i] = getPixel(
                            (c0.fA * u) + (c1.fA * v),
                            (c0.fR * u) + (c1.fR * v),
                            (c0.fG * u) + (c1.fG * v),
                            (c0.fB * u) + (c1.fB * v));
                }
            } else if (shaderMode == GShader::TileMode::kRepeat){
                for (int i = 0; i < count; ++i) {
                    local = fInverse.mapXY(x + i, y);
                    c = local.fX;
                    if (c < 0.0f) {
                        c = 1.0f - (c - GFloorToInt(c));
                    } else if (c > 1.0f){
                        c = c - GFloorToInt(c);
                    }
                    index = GFloorToInt(c * (gcCount - 1));
                    int rangeCount = gcCount - 1;
                    float rangeLength = 1.0f / (gcCount - 1);
                    while (c - rangeLength >= 0.0f){
                        c -= rangeLength;
                    }
                    u = c * rangeCount;
                    v = 1.0f - u;
                    c1 = gColors[index];
                    c0 = gColors[index + 1];
                    result[i] = getPixel(
                            (c0.fA * u) + (c1.fA * v),
                            (c0.fR * u) + (c1.fR * v),
                            (c0.fG * u) + (c1.fG * v),
                            (c0.fB * u) + (c1.fB * v));
                }
            } else {
                printf("Error at MyShader -- bad tile mode\n");
            }
        } else { // use bitmap
            int width = shaderBitmap.width();
            int height = shaderBitmap.height();
            if (shaderMode == GShader::TileMode::kClamp){
                GPoint local = fInverse.mapXY(x, y);
                int ix, iy;
                
                for (int i = 0; i < count; ++i) {
                    ix = GFloorToInt(local.x());
                    iy = GFloorToInt(local.y());
                    if (ix <= 0) {ix = 0;} else if (ix >= width) {ix = width - 1;}
                    if (iy <= 0) {iy = 0;} else if (iy >= height) {iy = height - 1;}
                    result[i] = *shaderBitmap.getAddr(ix, iy);
                    local.fX += fInverse[GMatrix::SX];
                    local.fY += fInverse[GMatrix::KY];
                }
            } else if (shaderMode == GShader::TileMode::kMirror){
                GPoint local = fInverse.mapXY(x, y);
                int ix, iy;
                for (int i = 0; i < count; ++i) {
                    ix = GFloorToInt(local.x());
                    iy = GFloorToInt(local.y());
                    float c = (float)ix / (float)(width-1);
                    if (c < 0.0f) {
                        if ((GFloorToInt(c) % 2) == 0) {c = 1.0f - (c - GFloorToInt(c));}
                        else {c = c - GFloorToInt(c);}
                        ix = GRoundToInt(c * (width-1));
                    } else {
                        if ((GFloorToInt(c) % 2) == 0) {c = c - GFloorToInt(c);}
                        else {c = 1.0f - (c - GFloorToInt(c));}
                        ix = GRoundToInt(c * (width-1));
                    }
                    c = (float)iy / (float)(height-1);
                    if (c < 0.0f) {
                        if ((GFloorToInt(c) % 2) == 0) {c = 1.0f - (c - GFloorToInt(c));}
                        else {c = c - GFloorToInt(c);}
                        iy = GRoundToInt(c * (height-1));
                    } else {
                        if ((GFloorToInt(c) % 2) == 0) {c = c - GFloorToInt(c);}
                        else {c = 1.0f - (c - GFloorToInt(c));}
                        iy = GRoundToInt(c * (height-1));
                    }
                    result[i] = *shaderBitmap.getAddr(ix, iy);
                    local.fX += fInverse[GMatrix::SX];
                    local.fY += fInverse[GMatrix::KY];
                }
            } else if (shaderMode == GShader::TileMode::kRepeat){
                GPoint local = fInverse.mapXY(x, y);
                int ix, iy;
                for (int i = 0; i < count; ++i) {
                    ix = GFloorToInt(local.x());
                    iy = GFloorToInt(local.y());
                    float c = (float)ix / (float)(width-1);
                    if (c < 0.0f) {
                        c = 1.0f - (c - GFloorToInt(c));
                        ix = GRoundToInt(c * (width-1));
                    } else if (c > 1.0f){
                        c = c - GFloorToInt(c);
                        ix = GRoundToInt(c * (width-1));
                    }
                    c = (float)iy / (float)(height-1);
                    if (c < 0.0f) {
                        c = c - GFloorToInt(c);
                        iy = GRoundToInt(c * (height-1));
                    } else if (c > 1.0f){
                        c = c - GFloorToInt(c);
                        iy = GRoundToInt(c * (height-1));
                    }
                    result[i] = *shaderBitmap.getAddr(ix, iy);
                    local.fX += fInverse[GMatrix::SX];
                    local.fY += fInverse[GMatrix::KY];
                }
            }
        }
    }
    
private:
    GMatrix fInverse;
    GColor* gColors;
    GBitmap shaderBitmap;
    GMatrix shaderLocalMatrix;
    GShader::TileMode shaderMode;
    int gcCount;
    
    GPixel getPixel(float fA, float fR, float fG, float fB){
        unsigned outA = (unsigned)GFloorToInt(fA * 255);
        unsigned outR = (unsigned)GFloorToInt(fR * 255); if (outR > outA) {outR = outA;}
        unsigned outG = (unsigned)GFloorToInt(fG * 255); if (outG > outA) {outG = outA;}
        unsigned outB = (unsigned)GFloorToInt(fB * 255); if (outB > outA) {outB = outA;}
        return GPixel_PackARGB(outA, outR, outG, outB);
    }
    
};
std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localM, GShader::TileMode mode){
    if (bitmap.pixels() == nullptr) {return nullptr;} // TODO: no way to compare localM to nullptr. how to check if invalid?
    return std::unique_ptr<GShader>(new MyShader(bitmap, localM, mode));
}
std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode mode) {
    if (count < 1) {return nullptr;}
    else {
        return std::unique_ptr<GShader>(new MyShader(p0, p1, colors, count, mode));
    }
}


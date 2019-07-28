
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GMath.h"
#include <stdio.h>

class TriColorShader : public GShader {
    
public:
    GPoint* pts;
    GColor* clr;
    GMatrix shaderLocalMatrix;
    GMatrix fInverse;
    
    TriColorShader(GPoint* points, GColor* colors){
        pts = points;
        clr = colors;
        shaderLocalMatrix.set6(points[1].fX-points[0].fX, points[2].fX-points[0].fX, points[0].fX,
                    points[1].fY-points[0].fY, points[2].fY-points[0].fY, points[0].fY);
    }

    // TODO
    bool isOpaque() override { return 0; }

    bool setContext(const GMatrix& ctm) {
        GMatrix temp;
        temp.setConcat(ctm, shaderLocalMatrix);
        return temp.invert(&fInverse);
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) {
        GPoint poi;
        GColor result;
        // P’x * C1 + P’y * C2 + (1 - P’x - P’y) * C0
        for (int i = 0; i < count; i++){
            poi = fInverse.mapXY(x + i, y);
            result.fA = poi.fX*(clr[1].fA - clr[0].fA) + poi.fY*(clr[2].fA - clr[0].fA) + clr[0].fA;
            result.fR = poi.fX*(clr[1].fR - clr[0].fR) + poi.fY*(clr[2].fR - clr[0].fR) + clr[0].fR;
            result.fG = poi.fX*(clr[1].fG - clr[0].fG) + poi.fY*(clr[2].fG - clr[0].fG) + clr[0].fG;
            result.fB = poi.fX*(clr[1].fB - clr[0].fB) + poi.fY*(clr[2].fB - clr[0].fB) + clr[0].fB;
            if (result.fA < 0.0f) {result.fA = 0.0f;} else if (result.fA > 1.0f) (result.fA = 1.0f); 
            if (result.fR < 0.0f) {result.fR = 0.0f;} else if (result.fR > result.fA) {result.fR = result.fA;}
            if (result.fG < 0.0f) {result.fG = 0.0f;} else if (result.fG > result.fA) {result.fG = result.fA;}
            if (result.fB < 0.0f) {result.fB = 0.0f;} else if (result.fB > result.fA) {result.fB = result.fA;}
            row[i] = GPixel_PackARGB(
                    (unsigned)GRoundToInt(result.fA * 255.0f),
                    (unsigned)GRoundToInt(result.fR * 255.0f),
                    (unsigned)GRoundToInt(result.fG * 255.0f),
                    (unsigned)GRoundToInt(result.fB * 255.0f)
                    );
        }
    }
};



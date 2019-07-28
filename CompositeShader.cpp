
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GPixel.h"
#include "ProxyShader.cpp"
#include "TriColorShader.cpp"

class CompositeShader : public GShader {
    ProxyShader sh1;
    TriColorShader sh2;
    GMatrix fInverse;
public:
    CompositeShader(ProxyShader& s1, TriColorShader& s2)
        : sh1(s1), sh2(s2) {}
        
    // TODO
    bool isOpaque() override { return 0; }
        
    bool setContext(const GMatrix& ctm) {
        return (sh1.setContext(ctm) & sh2.setContext(ctm));
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) {
        GPixel row1[count]; GPixel row2[count];
        sh1.shadeRow(x, y, count, row1);
        sh2.shadeRow(x, y, count, row2);
        for (int i = 0; i < count; i++){
            row[i] = GPixel_PackARGB(
                    (unsigned)GRoundToInt(GPixel_GetA(row1[i])*GPixel_GetA(row2[i])/255),
                    (unsigned)GRoundToInt(GPixel_GetR(row1[i])*GPixel_GetR(row2[i])/255),
                    (unsigned)GRoundToInt(GPixel_GetG(row1[i])*GPixel_GetG(row2[i])/255),
                    (unsigned)GRoundToInt(GPixel_GetB(row1[i])*GPixel_GetB(row2[i])/255));
        }
    }
        
};

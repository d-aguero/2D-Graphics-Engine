
#include "include/GMatrix.h"
#include "include/GShader.h"
#include <stdio.h>

class ProxyShader : public GShader {
    
public:
    GShader* fRealShader;
    GMatrix  fExtraTransform;
    
    ProxyShader(GShader* shader, const GMatrix& extraTransform)
        : fRealShader(shader), fExtraTransform(extraTransform) {}

    // TODO
    bool isOpaque() override { return 0; }

    bool setContext(const GMatrix& ctm) {
        GMatrix temp;
        temp.setConcat(ctm, fExtraTransform);
        if (fRealShader == nullptr) {printf("ProxyShader: This should not be reached"); return 0;}
        return fRealShader->setContext(temp);
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) {
        fRealShader->shadeRow(x, y, count, row);
    }
};



/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MyBlitter.h
 * Author: Alex
 *
 * Created on October 25, 2018, 2:55 AM
 */

#ifndef MYBLITTER_H
#define MYBLITTER_H

#include <float.h>
#include <stdio.h>
#include <stack>
#include "include/GCanvas.h"
#include "include/GPath.h"
#include "include/GBitmap.h"
#include "include/GPoint.h"
#include "include/GPixel.h"
#include "include/GColor.h"
#include "include/GRect.h"
#include "include/GMath.h"
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "MyEdge.h"

class MyBlitter {
public:
    MyBlitter();
    void blitShader(float x0, int y, float x1, GPixel* row, const GPaint& paint, const GBitmap& fDevice);
    void blitColor(float x0, int y, float x1, const GPaint& paint, const GBitmap& fDevice);
    void analyze(int width, int height, int y_0, float x_0, float x_1, const GBitmap& fDevice);
    unsigned getByte(unsigned S, unsigned D, unsigned Sa, unsigned Da, GBlendMode blendMode);
    void testBlitColor(float x0, int y, float x1, const GPaint& paint, const GBitmap& fDevice);
private:

};

#endif /* MYBLITTER_H */


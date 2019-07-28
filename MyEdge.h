/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MyEdge.h
 * Author: alex
 *
 * Created on September 19, 2018, 11:10 PM
 */

#ifndef MYEDGE_H
#define MYEDGE_H

#include "include/GPoint.h"
#include "include/GBitmap.h"

class MyEdge {
public:
    
    int ytop;
    int ybot;
    float m;
    float curr_x;
    
    MyEdge();
    
    void set(GPoint top, GPoint bottom);
    
    float getXIntercept(int curr_y);
    
    int getWindingValue();
    
    void divideEdges(MyEdge* input, int count, MyEdge* left, MyEdge* right);
    
    void getEdges(GPoint* points, int count, MyEdge* output);
    
    void sortEdges(MyEdge* edges, int count);
    
    int getCount(MyEdge* input, int count);
    
    void flipEdges(MyEdge* edges, int count);
    
    void clipEdges(MyEdge* input, int count, MyEdge* output, const GBitmap& fDevice);
    
    void setupEdges(GPoint points[3], MyEdge left[2], MyEdge right[2]);
    
private:

};

#endif /* MYEDGE_H */


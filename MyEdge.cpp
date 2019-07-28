/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MyEdge.cpp
 * Author: alex
 * 
 * Created on September 19, 2018, 11:10 PM
 */

#include <stdio.h>
#include "MyEdge.h"
#include "GMath.h"


    int ytop;
    int ybot;
    float m;
    float curr_x;
    
    MyEdge::MyEdge() {
        ytop = 0;
        ybot = 0;
        m = 0.0;
        curr_x = 0.0;
    }
    
    float MyEdge::getXIntercept(int curr_y){
        return (curr_x + (m * (float)(curr_y - ytop)));
    }
    
    void MyEdge::setupEdges(GPoint points[3], MyEdge left[2], MyEdge right[2]){
        
    }
    
    int MyEdge::getWindingValue(){
        if (ytop < ybot) {return 1;}
        else if (ytop > ybot) {return -1;}
        else {
            //printf("Invalid edge was fed to getWindingValue \n");
            return 0;
        }
    }
    
    void MyEdge::set(GPoint top, GPoint bottom){
        ytop = GRoundToInt(top.y());
        ybot = GRoundToInt(bottom.y());
        m = ( (bottom.x() - top.x()) / (bottom.y() - top.y()) );
        curr_x = top.x() + ( ((float)ytop - top.y()) * m ); // value of x at rounded int value of y0
    }
    
    void MyEdge::sortEdges(MyEdge* edges, int count){
        int i = 1;
        int j;
        float keyY;
        MyEdge keyEdge;
        while (i < count){
            keyY = edges[i].ytop;
            keyEdge = edges[i];
            j = i-1;
            while (j >= 0 && edges[j].ytop > keyY){
                edges[j+1] = edges[j];
                j--;
            }
            edges[j+1] = keyEdge;
            i++;
        }
    }
    
    void MyEdge::getEdges(GPoint* points, int count, MyEdge* output){
        for (int i=0; i<count-1; i++){
            if (points[i].y() != points[i+1].y()) {output[i].set(points[i], points[i+1]);}
        }
        if (points[count-1].y() != points[0].y()) {output[count-1].set(points[count-1], points[0]);}
    }
    
    void MyEdge::divideEdges(MyEdge* input, int count, MyEdge* left, MyEdge* right){
        int leftIndex = 0;
        int rightIndex = 0;
        for (int i = 0; i < count; i++){
            if (input[i].ytop == input[i].ybot) {
            } else if (input[i].ytop < input[i].ybot){
                left[leftIndex] = input[i];
                leftIndex++;
            } else if (input[i].ytop > input[i].ybot){
                right[rightIndex] = input[i];
                rightIndex++;
            }
        }
    }
    
    int MyEdge::getCount(MyEdge* input, int count){
        int returnCount = 0;
        for (int i = 0; i < count; i++){
            if (input[i].ytop == input[i].ybot) {returnCount = i; i = count;}
        }
        return returnCount;
    }
    
    void MyEdge::flipEdges(MyEdge* edges, int count) {
        GPoint p0, p1;
        for (int i = 0; i < count; i++){
            p0.set(edges[i].curr_x, edges[i].ytop);
            p1.set(edges[i].curr_x + ((edges[i].ybot - edges[i].ytop) * edges[i].m), edges[i].ybot);
            edges[i].set(p1, p0);
        }
    }
    
    void MyEdge::clipEdges(MyEdge* input, int count, MyEdge* output, const GBitmap& fDevice) {
        int i = 0; // for input array
        int k = 0; // for output array
        GPoint newP0, newP1;
        float p0y, p0x, p1y, p1x, m, h, w, temp;
        int bottom = input[count-1].ybot;
        int cont = 1;
        while (cont == 1) {
            if(input[i].ybot < 0) {} 
            else if (input[i].ytop >= fDevice.height()) {}
            else {
                p0y = (float)input[i].ytop;
                p0x = (float)input[i].curr_x;
                p1y = input[i].ybot;
                m = input[i].m;
                h = (float)fDevice.height();
                w = (float)fDevice.width();
                p1x = p0x + ((p1y - p0y) * m);
                //chop y
                if (GRoundToInt(p0y) < 0) {
                    p0x = p0x + ((0.0f - p0y) * m);
                    p0y = 0.0f;
                }
                if (GRoundToInt(p1y) >= fDevice.height()) {
                    p1x = p1x - ((p1y - h - 1.0f) * m);
                    p1y = h - 1.0f;
                }
                // chop OoB x's
                if (GRoundToInt(p0x) <= 0 && GRoundToInt(p1x) <= 0) {
                    if (GRoundToInt(p0y) < 0) {p0y = 0.0f;} if (GRoundToInt(p1y) >= fDevice.height()) {p1y = h-1.0f;}
                    newP0.set(0.0f, p0y);
                    newP1.set(0.0f, p1y);
                    output[k].set(newP0, newP1);
                    k++;
                } else if (p0x >= w && p1x >= w) {
                    if (GRoundToInt(p0y) < 0) {p0y = 0.0f;} if (GRoundToInt(p1y) >= fDevice.height()) {p1y = h-1.0f;}
                    newP0.set((w-1.0f), p0y);
                    newP1.set((w-1.0f), p1y);
                    output[k].set(newP0, newP1);
                    k++;
                } else {
                    // chop left / right x's
                    if (GRoundToInt(p0x) < 0) {
                        temp = p0y + ((p1y - p0y) * (0.0f - p0x) / (p1x - p0x)); //temp p1y
                        newP0.set(0.0f, p0y);
                        newP1.set(0.0f, temp);
                        output[k].set(newP0, newP1);
                        k++;
                        p0y = temp;
                        p0x = 0.0f;
                    } /*else if (p1x < 0.0f) {
                        temp = p1y - ((p1y - p0y) * (0.0f - p1x) / (p0x - p1x));
                        newP0.set(0.0f, temp);
                        newP1.set(0.0f, p1y);
                        output[k].set(newP0, newP1);
                        k++;
                        p1y = temp;
                        p1x = 0.0f;
                    }*/
                    if (GRoundToInt(p0x) >= fDevice.width()) {
                        temp = p0y + ((p1y - p0y) * (p0x - w - 1.0f) / (p0x - p1x)); //temp p1y
                        newP0.set((w - 1.0f), p0y);
                        newP1.set((w - 1.0f), temp);
                        output[k].set(newP0, newP1);
                        k++;
                        p0y = temp;
                        p0x = (w - 1.0f);
                    } /*else if (p1x >= w) {
                        temp = p1y - ((p1y - p0y) * (p1x - w - 1.0f) / (p1x - p0x)); //temp p1y
                        newP0.set((w - 1.0f), temp);
                        newP1.set((w - 1.0f), p1y);
                        output[k].set(newP0, newP1);
                        k++;
                        p1y = temp;
                        p1x = (w - 1.0f);
                    }*/
                    // remainder is completely within bounds of canvas
                    newP0.set(p0x, p0y);
                    newP1.set(p1x, p1y);
                    output[k].set(newP0, newP1);
                    k++;
                }
            }
            if (input[i].ybot == bottom || i == count-1) {
                cont = 0;//
            }
            i++;
        }
    }

/* 
 * File:   my_canvas.cpp
 * Author: alex
 * 
 * Created on September 3, 2018, 9:35 PM
 */

#include <float.h>
#include <stdio.h>
#include <stack>
#include <math.h>
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
#include "CompositeShader.cpp"
#include "MyEdge.h"
#include "MyBlitter.h"

class MyCanvas : public GCanvas {
public:
    
    MyCanvas(const GBitmap& device) : fDevice(device) {
        fctm.setIdentity();
        myStack.push(fctm);
    }
    
    MyEdge myEdge;
    MyBlitter blitter;
    GMatrix fctm;
    std::stack<GMatrix> myStack;
    
    /**
     *  Add contour(s) to the specified path that will draw a line from p0 to p1 with the specified
     *  width. Note that "width" is the distance from one side of the stroke to the other.
     *
     *  If roundCap is true, the path should also include a circular cap at each end of the line,
     *  where the circle has radius of width/2 and its center is positioned at p0 and p1.
     */
    void final_strokeLine(GPath* dst, GPoint p0, GPoint p1, float width, bool roundCap) {
        float m = (p1.x() - p0.x()) / (p1.y() - p0.y());
        float perpendicular_m = -1.0 * (p1.y() - p0.y()) / (p1.x() - p0.x());
        
        float delta_y = sqrt(pow(width/2, 2.0) / (1 + pow(perpendicular_m, 2.0)));
        
        float delta_x = sqrt(pow(width/2, 2.0) - pow(delta_y, 2.0));
        
        GPoint pts[4];
        pts[0].set(p0.x() - delta_x, p0.y() - delta_y);
        pts[1].set(p0.x() + delta_x, p0.y() + delta_y);
        pts[2].set(p1.x() - delta_x, p1.y() - delta_y);
        pts[3].set(p1.x() + delta_x, p1.y() + delta_y);
        
        dst->moveTo(pts[0]);
        dst->lineTo(pts[1]);
        dst->lineTo(pts[3]);
        dst->lineTo(pts[2]);
        
        if (roundCap){
            dst->addCircle(p0, width/2, GPath::kCCW_Direction);
            dst->addCircle(p1, width/2, GPath::kCCW_Direction);  
        }
    }

    /**
     *  Return a radial-gradient shader.
     *
     *  This is a shader defined by a circle with center point (cx, cy) and a radius.
     *  It supports an array colors (count >= 2) where
     *      color[0]       is the color at the center
     *      color[count-1] is the color at the outer edge of the circle
     *      the other colors (if any) are evenly distributed along the radius
     *
     *  e.g. If there are 4 colors and a radius of 90 ...
     *
     *      color[0] is at the center
     *      color[1] is at a distance of 30 from the center
     *      color[2] is at a distance of 60 from the center
     *      color[3] is at a distance of 90 from the center
     *
     *  Positions outside of the radius are clamped to color[count - 1].
     *  Positions inside the radius are linearly interpolated between the two nearest colors.
     *
     *  Interpolation occurs between GColors, and then it is premultiplied to a GPixel.
     */
    std::unique_ptr<GShader> final_createRadialGradient(GPoint center, float radius,
                                                                const GColor colors[], int count,
                                                                GShader::TileMode mode){
        return nullptr;
    }
    
    void save() override {
        GMatrix ctmCopy(fctm[0], fctm[1], fctm[2], fctm[3], fctm[4], fctm[5]);
        myStack.push(ctmCopy);
    }
    void restore() override {
        if (myStack.size() < 2) {}
        else {
            fctm = myStack.top();
            myStack.pop();
        }
    }
    void concat(const GMatrix& matrix) override {
        fctm.preConcat(matrix);
    }
    
    GMatrix compute_basis(GPoint p0, GPoint p1, GPoint p2){
        GMatrix matrix;
        matrix.set6(p1.fX-p0.fX, p2.fX-p0.fX, p0.fX,
                    p1.fY-p0.fY, p2.fY-p0.fY, p0.fY);
        return matrix;
    }
    
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint& paint) {
        // basically: implement drawpolygon, but specifically just use it for triangles.
        GPoint texPoints[3];
        GColor colorPoints[3];
        GPoint origPoints[3];
        int p1, p2, p3;
        int n = 0;
        for (int i = 0; i < count; i++){
            
            p1 = indices[n];
            p2 = indices[n+1];
            p3 = indices[n+2];
            origPoints[0] = verts[p1];
            origPoints[1] = verts[p2];
            origPoints[2] = verts[p3];
            if (texs == nullptr){} else {
                texPoints[0] = texs[p1];
                texPoints[1] = texs[p2];
                texPoints[2] = texs[p3];
            }
            if (colors == nullptr){} else {
                colorPoints[0] = colors[p1];
                colorPoints[1] = colors[p2];
                colorPoints[2] = colors[p3];
            }
            GMatrix temp;
            GMatrix texTemp;
            
            if (whichQuad == 1){
                texTemp = compute_basis(texPoints[0], texPoints[1], texPoints[2]);
                texTemp.invert(&texTemp);
                temp.setConcat(compute_basis(origPoints[0], origPoints[1], origPoints[2]), texTemp);
                ProxyShader texShader(paint.getShader(), temp);
                drawConvexPolygon_noclip(origPoints, 3, GPaint(&texShader));
            } else if (whichQuad == 2){
                TriColorShader colorShader(origPoints, colorPoints);
                drawConvexPolygon_noclip(origPoints, 3, GPaint(&colorShader));//
            } else if (colors != nullptr){
                TriColorShader colorShader(origPoints, colorPoints);
                if (texs != nullptr){
                    texTemp = compute_basis(texPoints[0], texPoints[1], texPoints[2]);
                    texTemp.invert(&texTemp);
                    
                    temp.setConcat(compute_basis(origPoints[0], origPoints[1], origPoints[2]), texTemp);
                    //                vvv(paint.getShader(), invT * P);
                    ProxyShader texShader(paint.getShader(), temp);
                    CompositeShader composite(texShader, colorShader);
                    drawConvexPolygon(origPoints, 3, GPaint(&composite));
                } else {
                    drawConvexPolygon(origPoints, 3, GPaint(&colorShader));//
                }
            } else {
                texTemp = compute_basis(texPoints[0], texPoints[1], texPoints[2]);
                texTemp.invert(&texTemp);
                temp.setConcat(compute_basis(origPoints[0], origPoints[1], origPoints[2]), texTemp);
                ProxyShader texShader(paint.getShader(), temp);
                drawConvexPolygon(origPoints, 3, GPaint(&texShader));
            }
            n += 3;
        }
        if (whichQuad != 0) {whichQuad == 0;}
    }
    
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
        
        int count = (level+1)*(level+1)*2;
        int vertCount = (level+2)*(level+2);
        
        int NEWindices[count*3];
        GPoint NEWverts[vertCount];
        GColor NEWcolors[vertCount];
        GPoint NEWtexs[vertCount];
        
        float step = 1.0f / ((float)level + 1.0f); 
        int index = 0;
        for (float v = 0; v <= 1.0f; v += step){
            for (float u = 0; u <= 1.0f; u += step){
                NEWverts[index] = ((1-u)*(1-v)*verts[0]) + ((1-v)*u*verts[1]) + ((1-u)*v*verts[3]) + (u*v*verts[2]);
                index++;}}
        if (colors != nullptr){
            index = 0;
            for (float v = 0; v <= 1.0f; v += step){
                for (float u = 0; u <= 1.0f; u += step){
                    NEWcolors[index].fA = ((1-u)*(1-v)*colors[0].fA) + ((1-v)*u*colors[1].fA) + ((1-u)*v*colors[3].fA) + (u*v*colors[2].fA);
                    NEWcolors[index].fR = ((1-u)*(1-v)*colors[0].fR) + ((1-v)*u*colors[1].fR) + ((1-u)*v*colors[3].fR) + (u*v*colors[2].fR);
                    NEWcolors[index].fG = ((1-u)*(1-v)*colors[0].fG) + ((1-v)*u*colors[1].fG) + ((1-u)*v*colors[3].fG) + (u*v*colors[2].fG);
                    NEWcolors[index].fB = ((1-u)*(1-v)*colors[0].fB) + ((1-v)*u*colors[1].fB) + ((1-u)*v*colors[3].fB) + (u*v*colors[2].fB);
                    index++;}}}
        if (texs != nullptr){
            index = 0;
            for (float v = 0; v <= 1.0f; v += step){
                for (float u = 0; u <= 1; u += step){
                    NEWtexs[index] = ((1-u)*(1-v)*texs[0]) + ((1-v)*u*texs[1]) + ((1-u)*v*texs[3]) + (u*v*texs[2]);
                    index++;}}}
        index = 0;
        for (int v = 0; v <= level; v++){
            for (int u = 0; u <= level; u++){
                NEWindices[index] = u + (level+2)*v;
                NEWindices[index+1] = u + (level+2)*(v+1);
                NEWindices[index+2] = (u+1) + (level+2)*v;
                
                NEWindices[index+3] = u + (level+2)*(v+1);
                NEWindices[index+4] = (u+1) + (level+2)*(v+1);
                NEWindices[index+5] = (u+1) + (level+2)*v;
                index += 6;}}
        if (texs == nullptr) {whichQuad = 2; drawMesh(NEWverts, NEWcolors, nullptr, count, NEWindices, paint);}
        else if (colors == nullptr) {whichQuad = 1; drawMesh(NEWverts, nullptr, NEWtexs, count, NEWindices, paint);}
        else {drawMesh(NEWverts, NEWcolors, NEWtexs, count, NEWindices, paint);}
    };
    
    void drawPath(const GPath& origPath, const GPaint& paint){
        if (origPath.countPoints() < 3) {}
        else {
            bool doShader = 0;
            GPath devPath = origPath;
            if (paint.getShader() != nullptr && paint.getShader()->setContext(fctm)) {doShader = 1;}
            
            devPath.transform(fctm);
            
            GPoint points[4];
            GPath::Edger edger = GPath::Edger(devPath);
            MyEdge edges[5000]; //MyEdge edges[devPath.countPoints()];
            int count = 0;
            GPath::Verb currVerb = edger.next(points);
            while (currVerb != GPath::Verb::kDone) {
                
                if (currVerb == GPath::Verb::kLine){
                    if (GRoundToInt(points[0].y()) != GRoundToInt(points[1].y())){      // avoids flat edges
                        edges[count].set(points[0], points[1]);
                        count++;
                    }
                } else if (currVerb == GPath::Verb::kQuad){
                    GPoint curvePoints[2];
                    curvePoints[0] = getQuad(points, 0);
                    for (float t = 0.02; t < 1; t += 0.02){
                        curvePoints[1] = getQuad(points, t);
                        if (GRoundToInt(curvePoints[0].y()) != GRoundToInt(curvePoints[1].y())){
                            edges[count].set(curvePoints[0], curvePoints[1]);
                            curvePoints[0] = curvePoints[1];
                            count++;
                        }
                    }
                } else if (currVerb == GPath::Verb::kCubic){
                    GPoint curvePoints[2];
                    curvePoints[0] = points[0];
                    for (float t = 0.02; t < 1; t += 0.02){
                        curvePoints[1] = getCubic(points, t);
                        if (GRoundToInt(curvePoints[0].y()) != GRoundToInt(curvePoints[1].y())){
                            edges[count].set(curvePoints[0], curvePoints[1]);
                            curvePoints[0] = curvePoints[1];
                            count++;
                        }
                    }
                } else {
                    printf("Error in drawPath -- bad verb\n");
                    return;
                }
                
                currVerb = edger.next(points);
            }
            
            // FOLLOWING CODE DOES NOT BOTHER WITH CLIPPING; does something else but stays in canvas bounds
            // TODO: if clipping is NECESSARY, then fix the code for YLoop_Path. 
            //       (if it isn't... maybe use this information to "adjust" convex clipper...)
            // TODO: add start_y and end_y to args for optimization
            YLoop_Path(edges, count, paint, doShader);
        }
    }
    
    void drawConvexPolygon_noclip(const GPoint* origPoints, int count, const GPaint& paint) {
        // used when you know clipping isn't needed.
        // a polygon can't be less than 3 points total
        if (count < 3) {}
        else {
            GShader* shader = paint.getShader();
            GPoint points[count];
            
            if (shader == nullptr){
                fctm.mapPoints(points, origPoints, count);
            } else if (!shader->setContext(fctm)) {return;} 
            else {fctm.mapPoints(points, origPoints, count);
            }
            MyEdge edges[count];
            // fetch edges, assuming they're provided in CCW order
            myEdge.getEdges(points, count, edges);
            
            MyEdge left[count];
            MyEdge right[count];
            myEdge.divideEdges(edges, count, left, right);
            
            int lcount = myEdge.getCount(left, count);
            int rcount = myEdge.getCount(right, count);
            
            if (right[0].ytop > right[0].ybot) {myEdge.flipEdges(right, rcount);}
            if (left[0].ytop > left[0].ybot) {myEdge.flipEdges(left, lcount);}
            myEdge.sortEdges(right, rcount);
            myEdge.sortEdges(left, lcount);
            
            int bottommost = right[rcount - 1].ybot;
            
            yLoop(left, right, bottommost, paint);
        }
    }
    
    void drawConvexPolygon(const GPoint* origPoints, int count, const GPaint& paint) {
        // a polygon can't be less than 3 points total
        if (count < 3) {}
        else {
            GShader* shader = paint.getShader();
            GPoint points[count];
            
            if (shader == nullptr){
                fctm.mapPoints(points, origPoints, count);
            } else if (!shader->setContext(fctm)) {return;} 
            else {fctm.mapPoints(points, origPoints, count);
            }
            MyEdge edges[count];
            // fetch edges, assuming they're provided in CCW order
            myEdge.getEdges(points, count, edges);
            
            MyEdge left[count];
            MyEdge right[count];
            myEdge.divideEdges(edges, count, left, right);
            
            int lcount = myEdge.getCount(left, count);
            int rcount = myEdge.getCount(right, count);
            
            if (right[0].ytop > right[0].ybot) {myEdge.flipEdges(right, rcount);}
            if (left[0].ytop > left[0].ybot) {myEdge.flipEdges(left, lcount);}
            myEdge.sortEdges(right, rcount);
            myEdge.sortEdges(left, lcount);
            
            int bottommost = right[rcount - 1].ybot;
            
            MyEdge leftClipped[count*3];
            MyEdge rightClipped[count*3];
            
            myEdge.clipEdges(left, lcount, leftClipped, fDevice);
            myEdge.clipEdges(right, rcount, rightClipped, fDevice);
            
            float left_bot_x = leftClipped[0].curr_x + ((leftClipped[0].ybot - leftClipped[0].ytop) * leftClipped[0].m);
            float right_bot_x = rightClipped[0].curr_x + ((rightClipped[0].ybot - rightClipped[0].ytop) * rightClipped[0].m);
            if (right_bot_x < left_bot_x){
                MyEdge temp[count*3];
                for (int i = 0; i < count*3; i++){
                    temp[i] = leftClipped[i];
                    leftClipped[i] = rightClipped[i];
                    rightClipped[i] = temp[i];
                }
            }
            
            yLoop(leftClipped, rightClipped, bottommost, paint);
        }
    }

    void drawPaint(const GPaint& paint) {
        
        if (paint.getShader() != nullptr) {
            GRect rect;
            rect.setLTRB(0, 0, this->fDevice.width() - 1, this->fDevice.height() - 1);
            drawRect(rect, paint);
        } else {
            int i = 0;
            int goal = this->fDevice.width() * this->fDevice.height();
            GPixel fillColor = GPixel_PackARGB(
                    GRoundToInt(paint.getAlpha() * 255), 
                    GRoundToInt((paint.getAlpha() * paint.getColor().fR) * 255), 
                    GRoundToInt((paint.getAlpha() * paint.getColor().fG) * 255), 
                    GRoundToInt((paint.getAlpha() * paint.getColor().fB) * 255));
            do {
                this->fDevice.pixels()[i] = fillColor;
                i++;
            } while (i < goal);
        }
    }
    
    void drawRect(const GRect& rect, const GPaint& paint) {
        if (GRoundToInt(rect.left()) >= GRoundToInt(rect.right()) || GRoundToInt(rect.top()) >= GRoundToInt(rect.bottom())) {}
        else if (GRoundToInt(rect.right()) <= 0 || GRoundToInt(rect.left()) >= this->fDevice.width()-1
                || GRoundToInt(rect.bottom()) <= 0 || GRoundToInt(rect.top()) >= this->fDevice.height()-1) {}
        else {
            GPoint points[4];
            points[0].set(rect.left(), rect.top());
            points[1].set(rect.left(), rect.bottom());
            points[2].set(rect.right(), rect.bottom());
            points[3].set(rect.right(), rect.top());
            drawConvexPolygon(points, 4, paint);
        }
        
    }
    
    
private:
    const GBitmap fDevice;
    int whichQuad;
    /*
    void drawTriangle(GPoint* points, const GPaint& paint){
        GShader* shader = paint.getShader();
        GPoint pts[3];
        if (shader == nullptr){
            fctm.mapPoints(pts, points, 3);
        } else if (!shader->setContext(fctm)) {return;} 
        else {fctm.mapPoints(pts, points, 3);
        }
        MyEdge edges[3];
        edges[0].set(pts[0], pts[1]);
        edges[1].set(pts[1], pts[2]);
        edges[2].set(pts[2], pts[0]);
        MyEdge left[2]; MyEdge right[2];
        int j = 0; int k = 0;
        int bottommost = 0;
        for (int i = 0; i < 3; i++){
            if (edges[i].ytop < edges[i].ybot){left[j] = edges[i]; j++;}
            else if (edges[i].ytop > edges[i].ybot){right[k] = edges[i]; k++;}
            if (GRoundToInt(pts[i].fY) > bottommost) {bottommost = GRoundToInt(pts[i].fY);}
        }
        myEdge.flipEdges(right, 2);
        myEdge.sortEdges(right, 2);
        myEdge.sortEdges(left, 2);
        yLoop(left, right, bottommost, paint);
    }*/
    
    GPoint getQuad(GPoint src[3], float t){
        GPoint result;
        result.set((1-t)*(1-t)*src[0].x() + 2*t*(1-t)*src[1].x() + t*t*src[2].x(),
                (1-t)*(1-t)*src[0].y() + 2*t*(1-t)*src[1].y() + t*t*src[2].y());
        return result;
    }

    GPoint getCubic(GPoint src[4], float t){
        GPoint result;
        result.set((1-t)*(1-t)*(1-t)*src[0].x() + 3*t*(1-t)*(1-t)*src[1].x() + 3*t*t*(1-t)*src[2].x() + t*t*t*src[3].x(),
                (1-t)*(1-t)*(1-t)*src[0].y() + 3*t*(1-t)*(1-t)*src[1].y() + 3*t*t*(1-t)*src[2].y() + t*t*t*src[3].y());
        return result;
    }
    
    void YLoop_Path(MyEdge* edges, int count, const GPaint& paint, bool doShader){
        
        int activeEdgeCount;
        float edgeX[count]; // used to store the x-intercept of curr_y and edges[i]
        int windingValue[count]; // used to store winding values; edges[index] has winding value windingValue[index]
        
        int right = fDevice.width() - 1;
        int bottom = fDevice.height() - 1;
        GShader* shader = paint.getShader();
        GPixel* row; row = new GPixel[this->fDevice.width()];
        int x0, x1, dx;
        
        for (int i = 0; i < bottom; i++){
            // fill edgeX and windingValue
            activeEdgeCount = 0;
            for (int j = 0; j < count; j++){
                if (((edges[j].ytop <= i && edges[j].ybot >= i) || (edges[j].ytop >= i && edges[j].ybot <= i)) && edges[j].ytop != edges[j].ybot
                        && (((i != edges[j].ybot && i != edges[j+1].ytop) && (i != edges[j].ytop || i != edges[j+1].ybot)) 
                            || edges[j].getWindingValue() != edges[j+1].getWindingValue())) {
                        edgeX[activeEdgeCount] = edges[j].getXIntercept(i);
                        windingValue[activeEdgeCount] = edges[j].getWindingValue();
                        activeEdgeCount++;
                    
                }
            }
            // sort x values ascending
            sortX(edgeX, activeEdgeCount);
            // we have all the info we need to start shading rows!
            int curr_winding = 0;
            for (int k = 0; k < activeEdgeCount-1; k++){
                curr_winding += windingValue[k];
                
                if (edgeX[k] == edgeX[k+1] && windingValue[k] == windingValue[k+1]){
                    printf("oof\n");
                    k += 1;
                }
                if (curr_winding != 0) {
                    if (GRoundToInt(edgeX[k]) >= right) {k = activeEdgeCount;}
                    else {
                        if (GRoundToInt(edgeX[k]) < 0){
                            if (GRoundToInt(edgeX[k+1]) < 0) {} // both are outside, do nothing
                            else if (GRoundToInt(edgeX[k]) == GRoundToInt(edgeX[k+1])) {} // same x intercept; nothing to draw
                            else if (GRoundToInt(edgeX[k+1]) > right) {
                                x0 = 0;
                                x1 = right;
                                dx = right;
                                if (doShader) {
                                    shader->shadeRow(x0, i, dx, row);
                                    blitter.blitShader(x0, i, x1, row, paint, this->fDevice);
                                } else {
                                    blitter.blitColor(x0, i, x1, paint, this->fDevice);
                                }
                                k = activeEdgeCount;
                            }
                            else {
                                x0 = 0;
                                x1 = GRoundToInt(edgeX[k+1]);
                                dx = x1;
                                if (doShader) {
                                    shader->shadeRow(x0, i, dx, row);
                                    blitter.blitShader(x0, i, x1, row, paint, this->fDevice);
                                } else {
                                    blitter.blitColor(x0, i, x1, paint, this->fDevice);
                                }
                            }
                        }
                        else { // edgeX[k] is within bounds of canvas
                            if (GRoundToInt(edgeX[k+1]) > right) {
                                x0 = GRoundToInt(edgeX[k]);
                                x1 = right;
                                dx = right - x0;
                                if (doShader) {
                                    shader->shadeRow(x0, i, dx, row);
                                    blitter.blitShader(x0, i, x1, row, paint, this->fDevice);
                                } else {
                                    blitter.blitColor(x0, i, x1, paint, this->fDevice);
                                }
                                k = activeEdgeCount;
                            }
                            else {
                                x0 = GRoundToInt(edgeX[k]);
                                x1 = GRoundToInt(edgeX[k+1]);
                                dx = GRoundToInt(edgeX[k+1] - edgeX[k]);
                                if (doShader) {
                                    shader->shadeRow(x0, i, dx, row);
                                    blitter.blitShader(x0, i, x1, row, paint, this->fDevice);
                                } else {
                                    blitter.blitColor(x0, i, x1, paint, this->fDevice);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    void sortX(float* xvals, int count){
        int i = 1;
        int j;
        float keyX;
        while (i < count){
            keyX = xvals[i];
            j = i-1;
            while (j >= 0 && xvals[j] > keyX){
                xvals[j+1] = xvals[j];
                j--;
            }
            xvals[j+1] = keyX;
            i++;
        }
    }
    
    void yLoop(MyEdge* left, MyEdge* right, int bottommost_point, const GPaint& paint){
        int i = 0; // index of left edges
        int k = 0; // index of right edges
        int curr_y = left[i].ytop;
        float curr_x0 = left[i].curr_x;
        float curr_x1 = right[k].curr_x;
        GShader* shader = paint.getShader();
        GPixel* row; row = new GPixel[this->fDevice.width()];
        
        while (curr_y < this->fDevice.height() && curr_y < bottommost_point) {
            if (curr_x0 < 0.0f) {curr_x0 = 0.0f;}
            if (GRoundToInt(curr_x1) >= this->fDevice.width()) {curr_x1 = (float)(this->fDevice.width() - 1);}
            
            if (GRoundToInt(curr_x0) < GRoundToInt(curr_x1)) {
                if (shader != nullptr) {
                    shader->shadeRow(GRoundToInt(curr_x0), curr_y, GRoundToInt(curr_x1 - curr_x0), row);
                    blitter.blitShader(curr_x0, curr_y, curr_x1, row, paint, this->fDevice);
                } else {
                    blitter.blitColor(curr_x0, curr_y, curr_x1, paint, this->fDevice);
                }
            }
            
            if (curr_y < left[i].ybot) {
                curr_x0 += left[i].m;
            } else {
                i++;
                curr_x0 = left[i].curr_x;
            }
            if (curr_y < right[k].ybot) {
                curr_x1 += right[k].m;
            } else {
                if (right[k].ybot != bottommost_point){
                    k++;
                    curr_x1 = right[k].curr_x;
                }
            }
            curr_y++;
        }
    }
};
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    if (!device.pixels()) {
        return nullptr;
    }
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}
void GDrawSomething_polys(GCanvas* canvas){
    GPath path;
    canvas->translate(100, 200);
    path.moveTo(0, 0).lineTo(50, 0).lineTo(-25, 40).lineTo(50, 80).lineTo(0, 80).lineTo(75, 40);
    canvas->drawPath(path, GPaint({1, 1, 0, 0}));
    for (int i = 0; i < 8; i++){
        canvas->translate(40, 0);
        if (i % 2 == 0) {canvas->translate(0, 20);}
        else {canvas->translate(0, -20);}
        canvas->drawPath(path, GPaint({1, (8.0f - (float)i) / 8.0f, (float)i / 8.0f, 0}));
    }
}
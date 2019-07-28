
#include "include/GPath.h"
#include "include/GMatrix.h"
#include <stdio.h>
#include <math.h>

#define PI 3.14159265

GPath& GPath::addCircle(GPoint center, float radius, Direction dir){
    float y = center.y();
    float x = center.x();
    float r = radius;
    float four = cos(PI/4);
    float eight = tan(PI/8);
    switch(dir){
        case Direction::kCCW_Direction:
            this->moveTo(GPoint::Make(x+r, y)); //start
            /*this->cubicTo(GPoint::Make(x+r, y+(r*bez)), GPoint::Make(x+(r*bez), y+r), GPoint::Make(x, y+r));
            this->cubicTo(GPoint::Make(x-(r*bez), y+r), GPoint::Make(x-r, y+(r*bez)), GPoint::Make(x-r, y));
            this->cubicTo(GPoint::Make(x-r, y-(r*bez)), GPoint::Make(x-(r*bez), y-r), GPoint::Make(x, y-r));
            this->cubicTo(GPoint::Make(x+(r*bez), y-r), GPoint::Make(x+r, y-(r*bez)), GPoint::Make(x+r, y));*/
            this->quadTo(GPoint::Make(x+r, y+(r*eight)), GPoint::Make(x+(r*four), y+(r*four))); // pi/4
            this->quadTo(GPoint::Make(x+(r*eight), y+r), GPoint::Make(x, y+r)); // pi/2
            this->quadTo(GPoint::Make(x-(r*eight), y+r), GPoint::Make(x-(r*four), y+(r*four))); // 3pi/4
            this->quadTo(GPoint::Make(x-r, y+(r*eight)), GPoint::Make(x-r, y)); // pi
            this->quadTo(GPoint::Make(x-r, y-(r*eight)), GPoint::Make(x-(r*four), y-(r*four))); // 5pi/4
            this->quadTo(GPoint::Make(x-(r*eight), y-r), GPoint::Make(x, y-r)); // 3pi/2
            this->quadTo(GPoint::Make(x+(r*eight), y-r), GPoint::Make(x+(r*four), y-(r*four))); // 7pi/4
            this->quadTo(GPoint::Make(x+r, y-(r*eight)), GPoint::Make(x+r, y)); // start
            return *this;
        case Direction::kCW_Direction:
            this->moveTo(GPoint::Make(x+r, y)); //start
            /*this->cubicTo(GPoint::Make(x+r, y-(r*bez)), GPoint::Make(x+(r*bez), y-r), GPoint::Make(x, y-r));
            this->cubicTo(GPoint::Make(x-(r*bez), y-r), GPoint::Make(x-r, y-(r*bez)), GPoint::Make(x-r, y));
            this->cubicTo(GPoint::Make(x-r, y+(r*bez)), GPoint::Make(x-(r*bez), y+r), GPoint::Make(x, y+r));
            this->cubicTo(GPoint::Make(x+(r*bez), y+r), GPoint::Make(x+r, y+(r*bez)), GPoint::Make(x+r, y));*/
            this->quadTo(GPoint::Make(x+r, y-(r*eight)), GPoint::Make(x+(r*four), y-(r*four))); // 7pi/4
            this->quadTo(GPoint::Make(x+(r*eight), y-r), GPoint::Make(x, y-r)); // 3pi/2
            this->quadTo(GPoint::Make(x-(r*eight), y-r), GPoint::Make(x-(r*four), y-(r*four))); // 5pi/4
            this->quadTo(GPoint::Make(x-r, y-(r*eight)), GPoint::Make(x-r, y)); // pi
            this->quadTo(GPoint::Make(x-r, y+(r*eight)), GPoint::Make(x-(r*four), y+(r*four))); // 3pi/4
            this->quadTo(GPoint::Make(x-(r*eight), y+r), GPoint::Make(x, y+r)); // pi/2
            this->quadTo(GPoint::Make(x+(r*eight), y+r), GPoint::Make(x+(r*four), y+(r*four))); // pi/4
            this->quadTo(GPoint::Make(x+r, y+(r*eight)), GPoint::Make(x+r, y)); // start
            return *this;
    }
    printf("Error at MyPath addCircle");
    return *this;
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t){
    // 1 --> 1 1 --> 1 2 1 --> 1 1 --> 1
    dst[0] = src[0];
    dst[1] = (1-t)*src[0] + t*src[1];
    dst[2] = (1-t)*(1-t)*src[0] + 2*t*(1-t)*src[1] + t*t*src[2];
    dst[3] = (1-t)*src[1] + t*src[2];
    dst[4] = src[2];
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t){
    // 1 --> 1 1 --> 1 2 1 --> 1 3 3 1 --> 1 2 1 --> 1 1 --> 1
    dst[0] = src[0];
    dst[1] = (1-t)*src[0] + t*src[1];
    dst[2] = (1-t)*(1-t)*src[0] + 2*t*(1-t)*src[1] + t*t*src[2];
    dst[3] = (1-t)*(1-t)*(1-t)*src[0] + 3*t*(1-t)*(1-t)*src[1] + 3*t*t*(1-t)*src[2] + t*t*t*src[3];
    dst[4] = (1-t)*(1-t)*src[1] + 2*t*(1-t)*src[2] + t*t*src[3];
    dst[5] = (1-t)*src[2] + t*src[3];
    dst[6] = src[3];
}

GPath& GPath::addRect(const GRect& r, Direction dir) {
    switch(dir){
        case Direction::kCCW_Direction:
            this->moveTo(r.left(), r.top());
            this->lineTo(r.left(), r.bottom());
            this->lineTo(r.right(), r.bottom());
            this->lineTo(r.right(), r.top());
            return *this;
        case Direction::kCW_Direction:
            this->moveTo(r.left(), r.top());
            this->lineTo(r.right(), r.top());
            this->lineTo(r.right(), r.bottom());
            this->lineTo(r.left(), r.bottom());
            return *this;
    }
    printf("Error at MyPath addRect");
    return *this;
}

GPath& GPath::addPolygon(const GPoint pts[], int count) {
    if (count > 2){
        this->moveTo(pts[0]);
        for (int i = 1; i < count; i++){
            this->lineTo(pts[i]);
        }
        return *this;
    } else {
        printf("Error at MyPath addPolygon");
        return *this;
    }
}

// TODO: implement this into code for optimization (though i don't think it'll optimize much...)
GRect GPath::bounds() const {
    int count = countPoints() + 0;
    if (count == 0){
        GRect empty;
        empty.setLTRB(0.0, 0.0, 0.0, 0.0);
        const GRect returnEmpty = empty;
        return returnEmpty;
    } else {
        float x[count], y[count];
        float minx = fPts[0].x(); float maxx = fPts[0].x(); float miny = fPts[0].y(); float maxy = fPts[0].y();
        for (int i = 1; i < count; i++){
            x[i] = fPts[i].x();
            y[i] = fPts[i].y();
            if (fPts[i].x() < minx) {minx = fPts[i].x();}
            if (fPts[i].x() > maxx) {maxx = fPts[i].x();}
            if (fPts[i].y() < miny) {miny = fPts[i].y();}
            if (fPts[i].y() > maxy) {maxy = fPts[i].y();}
        }
        GRect bounds;
        bounds.setLTRB(minx, miny, maxx, maxy);
        const GRect returnBounds = bounds;
        return returnBounds;
    }
}

void GPath::transform(const GMatrix& m) {
    GPoint points[fPts.size()];
    for (int i = 0; i < fPts.size(); i++){
        points[i] = fPts[i];
    }
    m.mapPoints(points, points, countPoints());
    for (int i = 0; i < fPts.size(); i++){
        fPts[i] = points[i];
    }
}
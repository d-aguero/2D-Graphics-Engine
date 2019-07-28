
#include "GMatrix.h"
#include <math.h>
#include <stdio.h>

/*
 *    SX   KX   TX,
 *    KY   SY   TY,
 *  [ Ax + By + C ]
 *  [ Dx + Ey + F ]
 */

void GMatrix::setIdentity() {
    //Set this matrix to identity.
    this->set6(1.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);
}

void GMatrix::setTranslate(float tx, float ty) {
    //Set this matrix to translate by the specified amounts.
    this->set6(1.0f, 0.0f, tx,
              0.0f, 1.0f, ty);
}

void GMatrix::setScale(float sx, float sy) {
    //Set this matrix to scale by the specified amounts.
    this->set6( sx , 0.0f, 0.0f,
              0.0f,  sy , 0.0f);
}

void GMatrix::setRotate(float radians) {
    //Set this matrix to rotate by the specified radians.
    this->set6(cos(radians), -sin(radians), 0,
               sin(radians),  cos(radians), 0);
}

void GMatrix::setConcat(const GMatrix& secundo, const GMatrix& primo) {
    /** Set this matrix to the concatenation of the two specified matrices, such that the resulting
     *  matrix, when applied to points will have the same effect as first applying the primo matrix
     *  to the points, and then applying the secundo matrix to the resulting points.
    */
    this->set6(
        (secundo.fMat[0] * primo.fMat[0]) + (secundo.fMat[1] * primo.fMat[3]),
        (secundo.fMat[0] * primo.fMat[1]) + (secundo.fMat[1] * primo.fMat[4]),
        (secundo.fMat[0] * primo.fMat[2]) + (secundo.fMat[1] * primo.fMat[5]) + secundo.fMat[2],
        (secundo.fMat[3] * primo.fMat[0]) + (secundo.fMat[4] * primo.fMat[3]),
        (secundo.fMat[3] * primo.fMat[1]) + (secundo.fMat[4] * primo.fMat[4]),
        (secundo.fMat[3] * primo.fMat[2]) + (secundo.fMat[4] * primo.fMat[5]) + secundo.fMat[5]);
}

bool GMatrix::invert(GMatrix* inverse) const {
    //If this matrix is invertible, return true and (if not null) set the inverse parameter.
    //If this matrix is not invertible, return false and ignore the inverse parameter.
    float a = this->fMat[0]; float b = this->fMat[1]; float c = this->fMat[2];
    float d = this->fMat[3]; float e = this->fMat[4]; float f = this->fMat[5];
    
    float det = (a * e) - (b * d);
    
    if (det == 0.0f) {return 0;}
    else {
        float div = (1 / det);
        inverse->set6(
            (e * div), (-b * div), ((b*f) - (e*c)) * div,
            (-d * div), (a * div), ((d*c) - (a*f)) * div);
        return 1;
    }
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    /**
     *  Transform the set of points in src, storing the resulting points in dst, by applying this matrix.
     * [ SX*xold + KX*yold + TX]
     * [ KY*xold + SY*yold + TY]
     */
    float sx, sy;
    for (int i = 0; i < count; i++){
        sx = src[i].x();
        sy = src[i].y();
        dst[i].fX = (this->fMat[0] * sx) + (this->fMat[1] * sy) + this->fMat[2];
        dst[i].fY = (this->fMat[3] * sx) + (this->fMat[4] * sy) + this->fMat[5];
    }
}
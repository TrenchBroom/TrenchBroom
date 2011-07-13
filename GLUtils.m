//
//  GLUtils.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GLUtils.h"

float const EdgeOffset = 0.0001f;

void glVertexV3f(const TVector3f* v) {
    glVertex3f(v->x, v->y, v->z);
}

void glSetEdgeOffset(float f) {
    glDepthRange(0.0, 1.0 - EdgeOffset * f);
}

void glResetEdgeOffset() {
    glDepthRange(EdgeOffset, 1.0);
}

void glBillboard(const TVector3f* cp, const TVector3f* cu, const TVector3f* bp, NSSize bs, float br) {
    TVector3f c, l, r;

    // billboard center
    c = *bp;
     
    // look vector from billboard center to camera position
    subV3f(cp, &c, &l);
    normalizeV3f(&l, &l);
    
    // right vector of billboard
    crossV3f(&l, cu, &r);
    
    // correct look vector
    crossV3f(cu, &r, &l);

//    float matrix[] = {r.x, r.y, r.z, 0, cu->x, cu->y, cu->z, 0, l.x, l.y, l.z, 0, bp->x, bp->y, bp->z, 1};
    float matrix[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, bp->x, bp->y, bp->z, 1};

    glTranslatef(-bs.width / 2, -bs.height / 2, 0);
//    glTranslatef(bp->x, bp->y, bp->z);
    glMultMatrixf(matrix);
    glTranslatef(bs.width / 2, bs.height / 2, 0);
}
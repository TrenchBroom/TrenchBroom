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

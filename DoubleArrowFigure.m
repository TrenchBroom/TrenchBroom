/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "DoubleArrowFigure.h"

#import "GLUtils.h"

static const int segments = 30;

@implementation DoubleArrowFigure

- (id)initWithDirection:(EAxis)theDirection shaftRadius:(float)theShaftRadius shaftLength:(float)theShaftLength headRadius:(float)theHeadRadius headLength:(float)theHeadLength {
    if ((self = [self init])) {
        scale = 1;
        
        shaftVertexCount = 2 * segments + 2;
        shaftVertices = malloc(shaftVertexCount * sizeof(TVector3f));
        shaftVertexNormals = malloc(shaftVertexCount * sizeof(TVector3f));
        makeCylinder(theShaftRadius, 2 * theShaftLength, segments, shaftVertices, shaftVertexNormals);
        
        // translate the shaft
        for (int i = 0; i < shaftVertexCount; i++)
            shaftVertices[i].z -= theShaftLength;

        headVertexCount = segments + 2;
        topHeadVertices = malloc(headVertexCount * sizeof(TVector3f));
        topHeadVertexNormals = malloc(headVertexCount * sizeof(TVector3f));
        bottomHeadVertices = malloc(headVertexCount * sizeof(TVector3f));
        bottomHeadVertexNormals = malloc(headVertexCount * sizeof(TVector3f));
        
        makeCone(theHeadRadius, theHeadLength, segments, topHeadVertices, topHeadVertexNormals);
        topHeadCapNormal = ZAxisNeg;
        topHeadCapPosition.x = 0;
        topHeadCapPosition.x = 0;
        topHeadCapPosition.x = theShaftLength;
        
        // translate the head to the top of the arrow and create bottom head
        for (int i = 0; i < headVertexCount; i++) {
            topHeadVertices[i].z += theShaftLength;
            bottomHeadVertices[i] = topHeadVertices[i];
            bottomHeadVertexNormals[i] = topHeadVertexNormals[i];
            bottomHeadVertices[i].y *= -1;
            bottomHeadVertices[i].z *= -1;
            bottomHeadVertexNormals[i].y *= -1;
            bottomHeadVertexNormals[i].z *= -1;
        }
        
        bottomHeadCapNormal = ZAxisPos;
        bottomHeadCapPosition.x = 0;
        bottomHeadCapPosition.x = 0;
        bottomHeadCapPosition.x = -theShaftLength;

        // rotate the arrow into the correct direction
        if (theDirection != A_Z) {
            EAxis rotAxis = theDirection == A_X ? A_Y : A_X;
            
            for (int i = 0; i < shaftVertexCount; i++) {
                rotate90CWV3f(&shaftVertices[i], rotAxis, &shaftVertices[i]);
                rotate90CWV3f(&shaftVertexNormals[i], rotAxis, &shaftVertexNormals[i]);
            }
            
            for (int i = 0; i < headVertexCount; i++) {
                rotate90CWV3f(&topHeadVertices[i], rotAxis, &topHeadVertices[i]);
                rotate90CWV3f(&topHeadVertexNormals[i], rotAxis, &topHeadVertexNormals[i]);
                rotate90CWV3f(&bottomHeadVertices[i], rotAxis, &bottomHeadVertices[i]);
                rotate90CWV3f(&bottomHeadVertexNormals[i], rotAxis, &bottomHeadVertexNormals[i]);
            }
            
            rotate90CWV3f(&topHeadCapNormal, rotAxis, &topHeadCapNormal);
            rotate90CWV3f(&topHeadCapPosition, rotAxis, &topHeadCapPosition);
            rotate90CWV3f(&bottomHeadCapNormal, rotAxis, &bottomHeadCapNormal);
            rotate90CWV3f(&bottomHeadCapPosition, rotAxis, &bottomHeadCapPosition);
        }

        // calculate surface normals and positions for silhouette tracing
        shaftSurfaceCount = segments;
        shaftSurfaceNormals = malloc((shaftSurfaceCount) * sizeof(TVector3f));
        shaftSurfacePositions = malloc((shaftSurfaceCount) * sizeof(TVector3f));
        
        for (int i = 0; i < shaftSurfaceCount; i++) {
            normV3f(&shaftVertices[2 * i], &shaftVertices[2 * i + 1], &shaftVertices[2 * i + 2], &shaftSurfaceNormals[i]);
            avg3V3f(&shaftVertices[2 * i], &shaftVertices[2 * i + 1], &shaftVertices[2 * i + 2], &shaftSurfacePositions[i]);
        }
        
        headSurfaceCount = headVertexCount - 2;
        topHeadSurfaceNormals = malloc(headSurfaceCount * sizeof(TVector3f));
        topHeadSurfacePositions = malloc(headSurfaceCount * sizeof(TVector3f));
        bottomHeadSurfaceNormals = malloc(headSurfaceCount * sizeof(TVector3f));
        bottomHeadSurfacePositions = malloc(headSurfaceCount * sizeof(TVector3f));
        
        for (int i = 0; i < headSurfaceCount; i++) {
            normV3f(&topHeadVertices[0], &topHeadVertices[i + 1], &topHeadVertices[i + 2], &topHeadSurfaceNormals[i]);
            avg3V3f(&topHeadVertices[0], &topHeadVertices[i + 1], &topHeadVertices[i + 2], &topHeadSurfacePositions[i]);
            normV3f(&bottomHeadVertices[0], &bottomHeadVertices[i + 1], &bottomHeadVertices[i + 2], &bottomHeadSurfaceNormals[i]);
            avg3V3f(&bottomHeadVertices[0], &bottomHeadVertices[i + 1], &bottomHeadVertices[i + 2], &bottomHeadSurfacePositions[i]);
        }
    }
    
    return self;
}

- (void)dealloc {
    free(shaftVertices);
    free(shaftVertexNormals);
    free(shaftSurfaceNormals);
    free(shaftSurfacePositions);
    free(topHeadVertices);
    free(topHeadVertexNormals);
    free(topHeadSurfaceNormals);
    free(topHeadSurfacePositions);
    free(bottomHeadVertices);
    free(bottomHeadVertexNormals);
    free(bottomHeadSurfaceNormals);
    free(bottomHeadSurfacePositions);
    [super dealloc];
}

- (void)render {
    float prevShaftDot, prevTopDot, prevBottomDot, curShaftDot, curTopDot, curBottomDot, topCapDot, bottomCapDot;
    TVector3f view, relativeCamPos;
    
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(2);
    glPointSize(2);
    
    subV3f(&cameraPosition, &position, &relativeCamPos);
    
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glScalef(scale, scale, scale);
    
    glColorV4f(&fillColor);
    
    // render shaft
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i < shaftVertexCount; i++) {
        glNormalV3f(&shaftVertexNormals[i]);
        glVertexV3f(&shaftVertices[i]);
    }
    glEnd();
    
    // render top head cap
    glBegin(GL_TRIANGLE_FAN);
    glNormalV3f(&topHeadCapNormal);
    for (int i = 1; i < headVertexCount ; i++)
        glVertexV3f(&topHeadVertices[i]);
    glEnd();
    
    // render top head
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < headVertexCount; i++) {
        glNormalV3f(&topHeadVertexNormals[i]);
        glVertexV3f(&topHeadVertices[i]);
    }
    glEnd();

    // render bottom head cap
    glBegin(GL_TRIANGLE_FAN);
    glNormalV3f(&bottomHeadCapNormal);
    for (int i = 1; i < headVertexCount; i++)
        glVertexV3f(&bottomHeadVertices[i]);
    glEnd();
    
    // render bottom head
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < headVertexCount; i++) {
        glNormalV3f(&bottomHeadVertexNormals[i]);
        glVertexV3f(&bottomHeadVertices[i]);
    }
    glEnd();
     
    
    // render the outlines
    glColorV4f(&outlineColor);
    glSetEdgeOffset(0.5f);

    // render shaft outline
    glBegin(GL_LINES);
    subV3f(&shaftSurfacePositions[shaftSurfaceCount - 1], &relativeCamPos, &view);
    prevShaftDot = dotV3f(&view, &shaftSurfaceNormals[shaftSurfaceCount - 1]);
    
    for (int i = 0; i < shaftSurfaceCount; i++) {
        subV3f(&shaftSurfacePositions[i], &relativeCamPos, &view);
        curShaftDot = dotV3f(&view, &shaftSurfaceNormals[i]);
        
        if (prevShaftDot > 0 && curShaftDot < 0) {
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 1]);

            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 2]);
            glVertexV3f(&shaftVertices[2 * i + 1]);
            glVertexV3f(&shaftVertices[2 * i + 3]);
        } else if (prevShaftDot < 0 && curShaftDot > 0) {
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 1]);
        } else if (curShaftDot < 0) {
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 2]);
            glVertexV3f(&shaftVertices[2 * i + 1]);
            glVertexV3f(&shaftVertices[2 * i + 3]);
        }

        prevShaftDot = curShaftDot;
    }
    
    glEnd();
    
    // render head outlines
    glBegin(GL_LINES);
    subV3f(&topHeadSurfacePositions[headSurfaceCount - 1], &relativeCamPos, &view);
    prevTopDot = dotV3f(&view, &topHeadSurfaceNormals[headSurfaceCount - 1]);
    
    subV3f(&bottomHeadSurfacePositions[headSurfaceCount - 1], &relativeCamPos, &view);
    prevBottomDot = dotV3f(&view, &bottomHeadSurfaceNormals[headSurfaceCount - 1]);

    subV3f(&topHeadCapPosition, &relativeCamPos, &view);
    topCapDot = dotV3f(&view, &topHeadCapNormal);
    
    subV3f(&bottomHeadCapPosition, &relativeCamPos, &view);
    bottomCapDot = dotV3f(&view, &bottomHeadCapNormal);
    
    for (int i = 0; i < headSurfaceCount; i++) {
        subV3f(&topHeadSurfacePositions[i], &relativeCamPos, &view);
        curTopDot = dotV3f(&view, &topHeadSurfaceNormals[i]);

        subV3f(&bottomHeadSurfacePositions[i], &relativeCamPos, &view);
        curBottomDot = dotV3f(&view, &bottomHeadSurfaceNormals[i]);
        
        if (prevTopDot > 0 && curTopDot < 0) {
            glVertexV3f(&topHeadVertices[i + 1]);
            glVertexV3f(&topHeadVertices[0]);
            
            glVertexV3f(&topHeadVertices[i + 1]);
            glVertexV3f(&topHeadVertices[i + 2]);
        } else if (prevTopDot < 0 && curTopDot > 0) {
            glVertexV3f(&topHeadVertices[i + 1]);
            glVertexV3f(&topHeadVertices[0]);
        } else if (curTopDot < 0) {
            glVertexV3f(&topHeadVertices[i + 1]);
            glVertexV3f(&topHeadVertices[i + 2]);
        } else if (topCapDot < 0) {
            glVertexV3f(&topHeadVertices[i + 1]);
            glVertexV3f(&topHeadVertices[i + 2]);
        }
        
        if (prevBottomDot > 0 && curBottomDot < 0) {
            glVertexV3f(&bottomHeadVertices[i + 1]);
            glVertexV3f(&bottomHeadVertices[0]);
            
            glVertexV3f(&bottomHeadVertices[i + 1]);
            glVertexV3f(&bottomHeadVertices[i + 2]);
        } else if (prevBottomDot < 0 && curBottomDot > 0) {
            glVertexV3f(&bottomHeadVertices[i + 1]);
            glVertexV3f(&bottomHeadVertices[0]);
        } else if (curBottomDot < 0) {
            glVertexV3f(&bottomHeadVertices[i + 1]);
            glVertexV3f(&bottomHeadVertices[i + 2]);
        } else if (bottomCapDot < 0) {
            glVertexV3f(&bottomHeadVertices[i + 1]);
            glVertexV3f(&bottomHeadVertices[i + 2]);
        }
        
        prevTopDot = curTopDot;
        prevBottomDot = curBottomDot;
    }
    glEnd();
    
    // render the head tips
    glBegin(GL_POINTS);
    glVertexV3f(&topHeadVertices[0]);
    glVertexV3f(&bottomHeadVertices[0]);
    glEnd();
    
    glPopMatrix();
    glEnable(GL_CULL_FACE);
    glLineWidth(1);
    glPointSize(1);
    glResetEdgeOffset();
}

- (void)setPosition:(const TVector3f *)thePosition {
    position = *thePosition;
}

- (void)setCameraPosition:(const TVector3f *)theCameraPosition {
    cameraPosition = *theCameraPosition;
}

- (void)setScale:(float)theScale {
    scale = theScale;
}

- (void)setFillColor:(const TVector4f *)theFillColor {
    NSAssert(theFillColor != NULL, @"fill color must not be NULL");
    fillColor = *theFillColor;
}

- (void)setOutlineColor:(const TVector4f *)theOutlineColor {
    NSAssert(theOutlineColor != NULL, @"outline color must not be NULL");
    outlineColor = *theOutlineColor;
}

@end
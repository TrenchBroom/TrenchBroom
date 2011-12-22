//
//  ArrowFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 17.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "ArrowFigure.h"
#import "GLUtils.h"

static const int segments = 30;
static const float shaftRadius = 2;
static const float shaftLength = 9;
static const float headRadius = 4;
static const float headLength = 7;

@implementation ArrowFigure

- (id)initWithDirection:(const TVector3f *)theDirection {
    if ((self = [super init])) {
        float cos;
        TVector3f axis;
        TQuaternion rot;
        
        shaftVertexCount = 2 * segments + 2;
        shaftVertices = malloc(shaftVertexCount * sizeof(TVector3f));
        shaftVertexNormals = malloc(shaftVertexCount * sizeof(TVector3f));
        makeCylinder(shaftRadius, shaftLength, segments, shaftVertices, shaftVertexNormals);
        
        shaftCapNormal = ZAxisNeg;
        shaftCapPosition = NullVector;
        
        headVertexCount = segments + 2;
        headVertices = malloc(headVertexCount * sizeof(TVector3f));
        headVertexNormals = malloc(headVertexCount * sizeof(TVector3f));
        
        makeCone(headRadius, headLength, segments, headVertices, headVertexNormals);
        headCapNormal = ZAxisNeg;
        headCapPosition.x = 0;
        headCapPosition.x = 0;
        headCapPosition.x = shaftLength;
        
        // translate the head to the top of the arrow
        for (int i = 0; i < headVertexCount; i++)
            headVertices[i].z += shaftLength;

        cos = dotV3f(&ZAxisPos, theDirection);
        if (cos < 1) {
            if (cos > -1) {
                crossV3f(&ZAxisPos, theDirection, &axis);
                normalizeV3f(&axis, &axis);
                setAngleAndAxisQ(&rot, acosf(cos), &axis);
                
                for (int i = 0; i < shaftVertexCount; i++) {
                    rotateQ(&rot, &shaftVertices[i], &shaftVertices[i]);
                    rotateQ(&rot, &shaftVertexNormals[i], &shaftVertexNormals[i]);
                }
                
                rotateQ(&rot, &shaftCapNormal, &shaftCapNormal);
                rotateQ(&rot, &shaftCapPosition, &shaftCapPosition);
                
                for (int i = 0; i < headVertexCount; i++) {
                    rotateQ(&rot, &headVertices[i], &headVertices[i]);
                    rotateQ(&rot, &headVertexNormals[i], &headVertexNormals[i]);
                }
                
                rotateQ(&rot, &headCapNormal, &headCapNormal);
                rotateQ(&rot, &headCapPosition, &headCapPosition);
            } else {
                for (int i = 0; i < shaftVertexCount; i++) {
                    shaftVertices[i].y *= -1;
                    shaftVertices[i].z *= -1;
                    shaftVertexNormals[i].y *= -1;
                    shaftVertexNormals[i].z *= -1;
                }
                
                shaftCapNormal.y *= -1;
                shaftCapNormal.z *= -1;
                shaftCapPosition.y *= -1;
                shaftCapPosition.z *= -1;
                
                for (int i = 0; i < headVertexCount; i++) {
                    headVertices[i].y *= -1;
                    headVertices[i].z *= -1;
                    headVertexNormals[i].y *= -1;
                    headVertexNormals[i].z *= -1;
                }
                
                headCapNormal.y *= -1;
                headCapNormal.z *= -1;
                headCapPosition.y *= -1;
                headCapPosition.z *= -1;
            }
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
        headSurfaceNormals = malloc(headSurfaceCount * sizeof(TVector3f));
        headSurfacePositions = malloc(headSurfaceCount * sizeof(TVector3f));
        
        for (int i = 0; i < headSurfaceCount; i++) {
            normV3f(&headVertices[0], &headVertices[i + 1], &headVertices[i + 2], &headSurfaceNormals[i]);
            avg3V3f(&headVertices[0], &headVertices[i + 1], &headVertices[i + 2], &headSurfacePositions[i]);
        }
    }
    
    return self;
}

- (void)dealloc {
    free(shaftVertices);
    free(shaftVertexNormals);
    free(shaftSurfaceNormals);
    free(shaftSurfacePositions);
    free(headVertices);
    free(headVertexNormals);
    free(headSurfaceNormals);
    free(headSurfacePositions);
    [super dealloc];
}

- (void)render {
    float prevDot, curDot, capDot;
    TVector3f view, relativeCamPos;
    
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(2);
    glPointSize(2);
    
    subV3f(&cameraPosition, &position, &relativeCamPos);
    
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    
    glColorV4f(&fillColor);
    
    // render shaft
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i < shaftVertexCount; i++) {
        glNormalV3f(&shaftVertexNormals[i]);
        glVertexV3f(&shaftVertices[i]);
    }
    glEnd();
    
    // shaft shaft cap
    glBegin(GL_TRIANGLE_FAN);
    glNormalV3f(&shaftCapNormal);
    for (int i = 0; i < shaftVertexCount / 4; i++)
        glVertexV3f(&shaftVertices[4 * i]);
    glEnd();
     
    // render head cap
    glBegin(GL_TRIANGLE_FAN);
    glNormalV3f(&headCapNormal);
    for (int i = 1; i < headVertexCount ; i++)
        glVertexV3f(&headVertices[i]);
    glEnd();
    
    // render head
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < headVertexCount; i++) {
        glNormalV3f(&headVertexNormals[i]);
        glVertexV3f(&headVertices[i]);
    }
    glEnd();
    
    
    // render the outlines
    glColorV4f(&outlineColor);
    glSetEdgeOffset(0.5f);
    
    // render shaft outline
    glBegin(GL_LINES);
    subV3f(&shaftSurfacePositions[shaftSurfaceCount - 1], &relativeCamPos, &view);
    prevDot = dotV3f(&view, &shaftSurfaceNormals[shaftSurfaceCount - 1]);
    
    subV3f(&shaftCapPosition, &relativeCamPos, &view);
    capDot = dotV3f(&view, &shaftCapNormal);

    for (int i = 0; i < shaftSurfaceCount; i++) {
        subV3f(&shaftSurfacePositions[i], &relativeCamPos, &view);
        curDot = dotV3f(&view, &shaftSurfaceNormals[i]);
        
        if (prevDot > 0 && curDot < 0) {
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 1]);
            
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 2]);
            glVertexV3f(&shaftVertices[2 * i + 1]);
            glVertexV3f(&shaftVertices[2 * i + 3]);
        } else if (prevDot < 0 && curDot > 0) {
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 1]);
        } else if (curDot < 0) {
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 2]);
            glVertexV3f(&shaftVertices[2 * i + 1]);
            glVertexV3f(&shaftVertices[2 * i + 3]);
        } else if (capDot < 0) {
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 2]);
        }
        
        prevDot = curDot;
    }
    
    glEnd();
    
    // render head outlines
    glBegin(GL_LINES);
    subV3f(&headSurfacePositions[headSurfaceCount - 1], &relativeCamPos, &view);
    prevDot = dotV3f(&view, &headSurfaceNormals[headSurfaceCount - 1]);
    
    subV3f(&headCapPosition, &relativeCamPos, &view);
    capDot = dotV3f(&view, &headCapNormal);
    
    for (int i = 0; i < headSurfaceCount; i++) {
        subV3f(&headSurfacePositions[i], &relativeCamPos, &view);
        curDot = dotV3f(&view, &headSurfaceNormals[i]);
        
        if (prevDot > 0 && curDot < 0) {
            glVertexV3f(&headVertices[i + 1]);
            glVertexV3f(&headVertices[0]);
            
            glVertexV3f(&headVertices[i + 1]);
            glVertexV3f(&headVertices[i + 2]);
        } else if (prevDot < 0 && curDot > 0) {
            glVertexV3f(&headVertices[i + 1]);
            glVertexV3f(&headVertices[0]);
        } else if (curDot < 0) {
            glVertexV3f(&headVertices[i + 1]);
            glVertexV3f(&headVertices[i + 2]);
        } else if (capDot < 0) {
            glVertexV3f(&headVertices[i + 1]);
            glVertexV3f(&headVertices[i + 2]);
        }
        
        prevDot = curDot;
    }
    glEnd();
    
    // render the head tip
    glBegin(GL_POINTS);
    glVertexV3f(&headVertices[0]);
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

- (void)setFillColor:(const TVector4f *)theFillColor {
    NSAssert(theFillColor != NULL, @"fill color must not be NULL");
    fillColor = *theFillColor;
}

- (void)setOutlineColor:(const TVector4f *)theOutlineColor {
    NSAssert(theOutlineColor != NULL, @"outline color must not be NULL");
    outlineColor = *theOutlineColor;
}

@end

//
//  ArrowFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 17.12.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "ArrowFigure.h"
#import "GLUtils.h"

static const int segments = 20;

@implementation ArrowFigure

- (id)init {
    if ((self = [super init])) {
        shaftVertexCount = 2 * segments + 2;
        shaftVertices = malloc(shaftVertexCount * sizeof(TVector3f));
        shaftNormals = malloc(shaftVertexCount * sizeof(TVector3f));
        makeCylinder(3, 10, segments, shaftVertices, shaftNormals);
        
        headVertexCount = segments + 1;
        headVertices = malloc(headVertexCount * sizeof(TVector3f));
        headNormals = malloc(headVertexCount * sizeof(TVector3f));
        makeCone(8, 10, 20, headVertices, headNormals);
        
        // translate the head to the top of the arrow
        for (int i = 0; i < headVertexCount; i++)
            headVertices[i].z += 10;
        
        cameraPos = NULL;
    }
    
    return self;
}

- (void)dealloc {
    free(shaftVertices);
    free(shaftNormals);
    free(headVertices);
    free(headNormals);
    cameraPos = NULL;
    [super dealloc];
}

- (void)render {
    float prevDot, curDot;
    TVector3f temp;
    
    NSAssert(cameraPos != NULL, @"camera position must not be NULL");
    
    glColorV4f(&fillColor);
    
    // render shaft cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    for (int i = 0; i < shaftVertexCount / 2 - 1; i++)
        glVertexV3f(&shaftVertices[2 * i]);
    glEnd();
    
    // render shaft
    glBegin(GL_QUADS);
    for (int i = 0; i < shaftVertexCount; i++) {
        glNormalV3f(&shaftNormals[i]);
        glVertexV3f(&shaftVertices[i]);
    }
    glEnd();
    
    // render head cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    for (int i = 1; i < headVertexCount; i++)
        glVertexV3f(&headVertices[i]);
    glEnd();
    
    // render head
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < headVertexCount; i++) {
        glNormalV3f(&headNormals[i]);
        glVertexV3f(&headVertices[i]);
    }
    glEnd();

    
    // render the outline
    glColorV4f(&outlineColor);
    
    // render shaft cap outline
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < shaftVertexCount / 2 - 1; i++)
        glVertexV3f(&shaftVertices[2 * i]);
    glEnd();
    
    // render shaft outline
    glBegin(GL_LINES);
    subV3f(&shaftVertices[0], cameraPos, &temp);
    prevDot = dotV3f(&temp, &shaftNormals[0]);
    
    for (int i = 2; i < shaftVertexCount / 2; i++) {
        subV3f(&shaftVertices[2 * i], cameraPos, &temp);
        curDot = dotV3f(&temp, &shaftNormals[2 * i]);
        
        if ((prevDot > 0) != (curDot > 0)) {
            glVertexV3f(&shaftVertices[2 * i]);
            glVertexV3f(&shaftVertices[2 * i + 1]);
        }
    }
    glEnd();

    // render head cap outline
    glBegin(GL_LINE_LOOP);
    glNormal3f(0, 0, -1);
    for (int i = 1; i < headVertexCount - 1; i++)
        glVertexV3f(&headVertices[i]);
    glEnd();
    
    // render head outline
    glBegin(GL_LINES);
    subV3f(&headVertices[1], cameraPos, &temp);
    prevDot = dotV3f(&temp, &headNormals[1]);
    
    for (int i = 2; i < headVertexCount; i++) {
        subV3f(&headVertices[i], cameraPos, &temp);
        curDot = dotV3f(&temp, &headNormals[i]);
        
        if ((prevDot > 0) != (curDot > 0)) {
            glVertexV3f(&headVertices[i]);
            glVertexV3f(&headVertices[0]);
        }
    }
    glEnd();
}

- (void)setCameraPosition:(const TVector3f *)theCameraPosition {
    cameraPos = theCameraPosition;
}

@end

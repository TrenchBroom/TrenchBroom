//
//  BrushToolCursor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "MoveCursor.h"

@implementation MoveCursor

- (void)setPlaneNormal:(EAxis)thePlaneNormal {
    planeNormal = thePlaneNormal;
}

- (void)renderArm {
    glTranslatef(0, 0, -15);
    gluCylinder(arms, 0, 4, 5, 20, 5);

    glTranslatef(0, 0, 5);
    gluQuadricOrientation(disks, GLU_OUTSIDE);
    gluDisk(disks, 0, 4, 10, 1);

    gluCylinder(arms, 2, 2, 20, 20, 1);
    
    glTranslatef(0, 0, 20);
    gluCylinder(arms, 4, 0, 5, 20, 5);
    gluQuadricOrientation(disks, GLU_INSIDE);
    gluDisk(disks, 0, 4, 20, 1);
}

- (void)render {
    if (!initialized) {
        arms = gluNewQuadric();
        disks = gluNewQuadric();
        gluQuadricDrawStyle(arms, GLU_FILL);
        gluQuadricDrawStyle(disks, GLU_FILL);
        gluQuadricNormals(arms, GLU_SMOOTH);
        gluQuadricNormals(disks, GLU_SMOOTH);
        initialized = YES;
    }
    
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    glFrontFace(GL_CCW);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    
    glColor4f(1, 1, 0, 1);
    if (planeNormal != A_X) {
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        [self renderArm];
        glPopMatrix();
    }
    
    if (planeNormal != A_Y) {
        glPushMatrix();
        glRotatef(270, 1, 0, 0);
        [self renderArm];
        glPopMatrix();
    }
    
    if (planeNormal != A_Z) {
        glPushMatrix();
        [self renderArm];
        glPopMatrix();
    }
    
    glPopMatrix();
    glFrontFace(GL_CW);
}

- (void)update:(TVector3f *)thePosition {
    position = *thePosition;
}

- (void)dealloc {
    if (initialized) {
        gluDeleteQuadric(arms);
        gluDeleteQuadric(disks);
    }
    [super dealloc];
}

@end

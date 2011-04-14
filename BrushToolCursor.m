//
//  BrushToolCursor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BrushToolCursor.h"

@implementation BrushToolCursor

- (void)setPlaneNormal:(EVectorComponent)thePlaneNormal {
    planeNormal = thePlaneNormal;
}

- (void)renderArm {
    glTranslatef(0, 0, -15);
    gluCylinder(arms, 0, 4, 5, 10, 5);

    glTranslatef(0, 0, 5);
    gluQuadricOrientation(disks, GLU_OUTSIDE);
    gluDisk(disks, 0, 4, 10, 1);

    gluCylinder(arms, 2, 2, 20, 10, 1);
    
    glTranslatef(0, 0, 20);
    gluCylinder(arms, 4, 0, 5, 10, 5);
    gluQuadricOrientation(disks, GLU_INSIDE);
    gluDisk(disks, 0, 4, 10, 1);
}

- (void)render {
    if (!initialized) {
        arms = gluNewQuadric();
        disks = gluNewQuadric();
        gluQuadricDrawStyle(arms, GLU_FILL);
        gluQuadricDrawStyle(disks, GLU_FILL);
        initialized = YES;
    }
    
    glDisable(GL_TEXTURE_2D);
    glFrontFace(GL_CCW);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glTranslatef([position x], [position y], [position z]);
    
    glColor4f(1, 1, 0, 1);
    if (planeNormal != VC_X) {
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        [self renderArm];
        glPopMatrix();
    }
    
    if (planeNormal != VC_Y) {
        glPushMatrix();
        glRotatef(270, 1, 0, 0);
        [self renderArm];
        glPopMatrix();
    }
    
    if (planeNormal != VC_Z) {
        glPushMatrix();
        [self renderArm];
        glPopMatrix();
    }
    
    glPopMatrix();
    
    glFrontFace(GL_CW);
}

- (void)update:(Vector3f *)thePosition {
    [position release];
    position = [thePosition retain];
}

- (void)dealloc {
    if (initialized) {
        gluDeleteQuadric(arms);
        gluDeleteQuadric(disks);
    }
    [position release];
    [super dealloc];
}

@end

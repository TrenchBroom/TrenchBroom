//
//  BrushToolFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "CompassFigure.h"
#import "Camera.h"
#import "Vector3f.h"

@implementation CompassFigure
- (id)init {
    if (self = [super init]) {
        arms = gluNewQuadric();
        disks = gluNewQuadric();
        gluQuadricOrientation(disks, GLU_INSIDE);
    }
    
    return self;
}

- (id)initWithCamera:(Camera *)theCamera {
    if (self = [super init]) {
        camera = [theCamera retain];
    }
    
    return self;
}

- (void)render {
    glFrontFace(GL_CCW);
    glPushMatrix();
    glTranslatef([position x], [position y], [position z]);
    
    if (drawX) {
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        
        glColor4f(1, 0, 0, 0.6f);
        gluCylinder(arms, 4, 4, 30, 10, 1);
        gluDisk(disks, 0, 4, 10, 1);
        glTranslatef(0, 0, 30);
        gluCylinder(arms, 8, 0, 12, 10, 5);
        gluDisk(disks, 0, 8, 10, 1);
        
        glPopMatrix();
    }
    
    if (drawY) {
        glPushMatrix();
        glRotatef(270, 1, 0, 0);
        
        glColor4f(0, 1, 0, 0.6f);
        gluCylinder(arms, 4, 4, 30, 10, 1);
        gluDisk(disks, 0, 4, 10, 1);
        glTranslatef(0, 0, 30);
        gluCylinder(arms, 8, 0, 12, 10, 5);
        gluDisk(disks, 0, 8, 10, 1);
        
        glPopMatrix();
    }
    
    if (drawZ) {
        glColor4f(0, 0, 1, 0.6f);
        gluCylinder(arms, 4, 4, 30, 10, 1);
        gluDisk(disks, 0, 4, 10, 1);
        glTranslatef(0, 0, 30);
        gluCylinder(arms, 8, 0, 12, 10, 5);
        gluDisk(disks, 0, 8, 10, 1);
    }
    
    glPopMatrix();
    glFrontFace(GL_CW);
}

- (void)dealloc {
    gluDeleteQuadric(arms);
    gluDeleteQuadric(disks);
    [camera release];
    [super dealloc];
}

@end

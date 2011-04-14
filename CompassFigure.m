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

- (id)initWithCamera:(Camera *)theCamera {
    if (self = [self init]) {
        camera = [theCamera retain];
    }
    
    return self;
}

- (void)renderArm {
    glTranslatef(0, 0, -30);
    gluCylinder(arms, 4, 4, 60, 10, 1);
    gluDisk(disks, 0, 4, 10, 1);
    glTranslatef(0, 0, 60);
    gluCylinder(arms, 8, 0, 12, 10, 5);
    gluDisk(disks, 0, 8, 10, 1);
}

- (void)render {
    if (!initialized) {
        arms = gluNewQuadric();
        disks = gluNewQuadric();
        gluQuadricDrawStyle(arms, GLU_FILL);
        gluQuadricDrawStyle(disks, GLU_FILL);
        gluQuadricOrientation(disks, GLU_INSIDE);
        initialized = YES;
    }
    
    glDisable(GL_TEXTURE_2D);
    glFrontFace(GL_CCW);
    
    glPushAttrib(GL_VIEWPORT_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); {
        glViewport(0, 0, 100, 100);
        glLoadIdentity();
        // gluPerspective([camera fieldOfVision], 1, 1, 100);
        glOrtho(-50, 50, -50, 50, 1, 100);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix(); {
            Vector3f* direction = [camera direction];
            Vector3f* up = [camera up];
            Vector3f* position = [[Vector3f alloc] initWithFloatVector:direction];
            [position scale:-50];

            glLoadIdentity();
            gluLookAt([position x],
                      [position y],
                      [position z],
                      [position x] + [direction x],
                      [position y] + [direction y],
                      [position z] + [direction z],
                      [up x],
                      [up y],
                      [up z]);
            [position release];

            glColor4f(1, 0, 0, 1);
            glPushMatrix();
            glRotatef(90, 0, 1, 0);
            [self renderArm];
            glPopMatrix();
            
            glColor4f(0, 1, 0, 1);
            glPushMatrix();
            glRotatef(270, 1, 0, 0);
            [self renderArm];
            glPopMatrix();
            
            glColor4f(0, 0, 1, 1);
            glPushMatrix();
            [self renderArm];
            glPopMatrix();
        }
        glPopMatrix();
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();

    glFrontFace(GL_CW);
}

- (void)dealloc {
    if (initialized) {
        gluDeleteQuadric(arms);
        gluDeleteQuadric(disks);
    }
    [camera release];
    [super dealloc];
}

@end

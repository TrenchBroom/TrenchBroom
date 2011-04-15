//
//  DragFaceCursor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DragFaceCursor.h"
#import "Vector3f.h"
#import "math.h"

@implementation DragFaceCursor

- (id)init {
    if (self = [super init]) {
        axis = [[Vector3f alloc] init];
    }
    return self;
}

- (void)render {
    if (!initialized) {
        arm = gluNewQuadric();
        disks = gluNewQuadric();
        gluQuadricDrawStyle(arm, GLU_FILL);
        gluQuadricDrawStyle(disks, GLU_FILL);
        gluQuadricOrientation(disks, GLU_OUTSIDE);
        initialized = YES;
    }
    
    glDisable(GL_TEXTURE_2D);
    glFrontFace(GL_CCW);
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glTranslatef([position x], [position y], [position z]);
    glRotatef(angle, [axis x], [axis y], [axis z]);
    
    glColor4f(1, 1, 0, 1);
    gluDisk(disks, 0, 2, 10, 1);
    gluCylinder(arm, 2, 2, 10, 10, 1);
    glTranslatef(0, 0, 10);
    gluDisk(disks, 0, 4, 10, 1);
    gluCylinder(arm, 4, 0, 5, 10, 5);
    
    glPopMatrix();
    glFrontFace(GL_CW);
}

- (void)setDragDir:(Vector3f *)theDragDir {
    [axis setFloat:[Vector3f zAxisPos]];
    float cos = [axis dot:theDragDir];
    
    if (feq(1, cos)) {
        angle = 0;
        [axis setFloat:[Vector3f nullVector]];
    } else if (feq(-1, cos)) {
        angle = 180;
        [axis setFloat:[Vector3f xAxisPos]];
    } else {
        angle = acos(cos) / (2 * M_PI) * 360;
        [axis cross:theDragDir];
        [axis normalize];
    }
}

- (void)update:(Vector3f *)thePosition {
    [position release];
    position = [thePosition retain];
}

- (void)dealloc {
    if (initialized) {
        gluDeleteQuadric(arm);
        gluDeleteQuadric(disks);
    }
    [position release];
    [axis release];
    [super dealloc];
}

@end

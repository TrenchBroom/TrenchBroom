//
//  DragFaceCursor.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DragFaceCursor.h"
#import "math.h"

@implementation DragFaceCursor

- (void)render {
    if (!initialized) {
        arm = gluNewQuadric();
        disks = gluNewQuadric();
        gluQuadricDrawStyle(arm, GLU_FILL);
        gluQuadricDrawStyle(disks, GLU_FILL);
        gluQuadricOrientation(disks, GLU_INSIDE);
        gluQuadricNormals(arm, GLU_SMOOTH);
        gluQuadricNormals(disks, GLU_SMOOTH);
        initialized = YES;
    }
    
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    glFrontFace(GL_CCW);
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glRotatef(angle, axis.x, axis.y, axis.z);
    
    glColor4f(1, 1, 0, 1);
    gluDisk(disks, 0, 2, 20, 1);
    gluCylinder(arm, 2, 2, 10, 20, 1);
    glTranslatef(0, 0, 10);
    gluDisk(disks, 0, 4, 20, 1);
    gluCylinder(arm, 4, 0, 8, 20, 5);
    
    glPopMatrix();
    glFrontFace(GL_CW);
}

- (void)setDragDir:(TVector3f *)theDragDir {
    float cos = dotV3f(&ZAxisPos, theDragDir);
    
    if (feq(1, cos)) {
        angle = 0;
        axis = NullVector;
    } else if (feq(-1, cos)) {
        angle = 180;
        axis = XAxisPos;
    } else {
        angle = acos(cos) / (2 * M_PI) * 360;
        crossV3f(&ZAxisPos, theDragDir, &axis);
        normalizeV3f(&axis, &axis);
    }
}

- (void)update:(TVector3f *)thePosition {
    position = *thePosition;
}

- (void)dealloc {
    if (initialized) {
        gluDeleteQuadric(arm);
        gluDeleteQuadric(disks);
    }
    [super dealloc];
}

@end

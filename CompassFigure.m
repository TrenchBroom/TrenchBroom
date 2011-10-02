/*
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

#import "CompassFigure.h"
#import "Camera.h"

@implementation CompassFigure

- (id)initWithCamera:(Camera *)theCamera {
    if (self = [self init]) {
        camera = [theCamera retain];
    }
    
    return self;
}

- (void)renderArm {
    glTranslatef(0, 0, -10);
    gluCylinder(arms, 1, 1, 20, 10, 1);
    gluDisk(disks, 0, 1, 10, 1);
    glTranslatef(0, 0, 20);
    gluCylinder(arms, 2, 0, 3, 10, 5);
    gluDisk(disks, 0, 2, 10, 1);
}

- (void)render {
    if (!initialized) {
        arms = gluNewQuadric();
        disks = gluNewQuadric();
        gluQuadricDrawStyle(arms, GLU_FILL);
        gluQuadricDrawStyle(disks, GLU_FILL);
        gluQuadricOrientation(disks, GLU_INSIDE);
        gluQuadricNormals(arms, GLU_SMOOTH);
        gluQuadricNormals(disks, GLU_SMOOTH);
        initialized = YES;
    }
    
    glPolygonMode(GL_FRONT, GL_FILL);
    
    EAxis axis = strongestComponentV3f([camera direction]);
    
    // X axis
    if (axis == A_X)
        glColor4f(0.8f, 0.8f, 0.8f, 1);
    else
        glColor4f(1, 0, 0, 1);
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    [self renderArm];
    glPopMatrix();
    
    // Y axis
    if (axis == A_Y)
        glColor4f(0.8f, 0.8f, 0.8f, 1);
    else
        glColor4f(0, 1, 0, 1);
    glPushMatrix();
    glRotatef(270, 1, 0, 0);
    [self renderArm];
    glPopMatrix();

    // Z axis
    if (axis == A_Z)
        glColor4f(0.8f, 0.8f, 0.8f, 1);
    else
        glColor4f(0, 0, 1, 1);
    glPushMatrix();
    [self renderArm];
    glPopMatrix();
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

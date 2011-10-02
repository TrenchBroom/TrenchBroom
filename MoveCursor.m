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

- (void)update:(const TVector3f *)thePosition {
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

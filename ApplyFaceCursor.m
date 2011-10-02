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

#import "ApplyFaceCursor.h"
#import "Face.h"

@implementation ApplyFaceCursor

- (void)render {
    glDisable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_LINE);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glTranslatef(position.x - center.x, position.y - center.y, position.z - center.z);
    glMultMatrixf(matrix.values);

    glLineWidth(2);
    glColor4f(1, 1, 0, 1);

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(-8, -8, 0);
    glVertex3f(-8, +8, 0);
    glVertex3f(+8, +8, 0);
    glVertex3f(+8, -8, 0);
    glEnd();
    
    if (applyFlags) {
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex3f(-6, 6, 0);
        glVertex3f(+2, 6, 0);
        glVertex3f(-6, 4, 0);
        glVertex3f(+5, 4, 0);
        glVertex3f(-6, 2, 0);
        glVertex3f(+5, 2, 0);
        glVertex3f(-6, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(-6, -2, 0);
        glVertex3f(3, -2, 0);
        glVertex3f(-6, -4, 0);
        glVertex3f(5, -4, 0);
        glVertex3f(-6, -6, 0);
        glVertex3f(1, -6, 0);
        glEnd();
    }
    
    glLineWidth(1);
    glPopMatrix();
    
}

- (void)setFace:(id <Face>)theFace {
    matrix = *[theFace surfaceToWorldMatrix];
    center = *[theFace center];
}

- (void)setApplyFlags:(BOOL)doApplyFlags {
    applyFlags = doApplyFlags;
}

- (void)update:(const TVector3f *)thePosition {
    position = *thePosition;
}

@end

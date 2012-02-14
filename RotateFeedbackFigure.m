/*
Copyright (C) 2010-2012 Kristian Duske

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

#import "RotateFeedbackFigure.h"

@implementation RotateFeedbackFigure

- (void)render {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);
    glEnable(GL_DEPTH_TEST);

    glDisable(GL_DEPTH_TEST);
    glColor4f(0, 1, 0, 1);

    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 100);
    glVertex3f(0, 0, 0);
    glVertex3f(100 * vAxis.x, 100 * vAxis.y, 100 * vAxis.z);
    glEnd();

    glPopMatrix();
}

- (void)setCenter:(const TVector3f *)theCenter radius:(float)theRadius {
    center = *theCenter;
    radius = theRadius;
}

- (void)setVerticalAxis:(const TVector3f *)theVerticalAxis {
    vAxis = *theVerticalAxis;
}

- (void)setDragging:(BOOL)isDragging {
    drag = isDragging;
}

- (void)setHorizontalAngle:(float)theHAngle verticalAngle:(float)theVAngle {
    hAngle = theHAngle;
    vAngle = theVAngle;
}

@end

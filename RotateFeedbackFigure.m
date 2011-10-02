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

#import "RotateFeedbackFigure.h"

@implementation RotateFeedbackFigure

- (void)render {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);
    glEnable(GL_DEPTH_TEST);

    glRotatef((initialHAngle + hAngle) * 360 / (2 * M_PI), 0, 0, 1);
    
    int circleSegments = 2 * radius;
    if (circleSegments < 12)
        circleSegments = 12;
    TVector3f circle[circleSegments];
    makeCircle(radius, circleSegments, circle);
    
    glDisable(GL_DEPTH_TEST);
    glColor4f(1, 1, 0, 0.3f);
    
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(radius, 0, 0);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();
    
    glRotatef(90, 1, 0, 0);
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glColor4f(1, 1, 0, 1);

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();
    
    glRotatef(90, 1, 0, 0);
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < circleSegments; i++)
        glVertex3f(circle[i].x, circle[i].y, circle[i].z);
    glEnd();
    
    glPopMatrix();
}

- (void)updateCenter:(TVector3f *)theCenter radius:(float)theRadius verticalAxis:(EAxis)theVerticalAxis initialHAngle:(float)theInitialHAngle initialVAngle:(float)theInitialVAngle {
    NSAssert(vAxis != A_Z, @"vertical axis must not be the Z axis");
    
    center = *theCenter;
    radius = fmax(30, theRadius);
    vAxis = theVerticalAxis;
    initialHAngle = theInitialHAngle;
    initialVAngle = theInitialVAngle;
}

- (void)setDragging:(BOOL)isDragging {
    drag = isDragging;
}

- (void)updateHorizontalAngle:(float)theHAngle verticalAngle:(float)theVAngle {
    hAngle = theHAngle;
    vAngle = theVAngle;
}

@end

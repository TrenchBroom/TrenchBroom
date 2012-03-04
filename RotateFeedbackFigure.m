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
#import "GLUtils.h"
#import "Grid.h"

@implementation RotateFeedbackFigure

- (id)initWithGrid:(Grid *)theGrid {
    NSAssert(theGrid != nil, @"grid must not be nil");
    
    if ((self = [self init])) {
        grid = theGrid;
    }
    
    return self;
}

- (void)render:(id <Filter>)theFilter {
    int segments = radius;
    TVector3f points[segments];
    makeCircle(radius, segments, points);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);

    glColor4f(1, 1, 1, 0.1f);
    glBegin(GL_TRIANGLE_FAN);
    glVertexV3f(&NullVector);
    for (int i = 0; i < segments; i++)
        glVertexV3f(&points[i]);
    glVertexV3f(&points[0]);
    glEnd();
    
    glColor4f(1, 1, 1, 0.7f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
        glVertexV3f(&points[i]);
    glEnd();

    glPushMatrix();
    float angle = acosf(dotV3f(&vAxis, &XAxisPos));
    if (!isnan(angle)) {
        TVector3f cross;
        crossV3f(&vAxis, &XAxisPos, &cross);
        if (cross.z > 0)
            angle *= -1;
        
        glRotatef(angle * 180 / M_PI, 0, 0, 1);
    }
    
    glRotatef(90, 1, 0, 0);
    
    glColor4f(1, 1, 1, 0.1f);
    glBegin(GL_TRIANGLE_FAN);
    glVertexV3f(&NullVector);
    for (int i = 0; i < segments; i++)
        glVertexV3f(&points[i]);
    glVertexV3f(&points[0]);
    glEnd();

    glColor4f(1, 1, 1, 0.7f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
        glVertexV3f(&points[i]);
    glEnd();

    glPopMatrix();
    
    glBegin(GL_LINES);
    if (!drag && [grid snap]) {
        float x,y;
        float a = 0;
        int n = roundf(2 * M_PI / [grid actualRotAngle]);
        float da = 2 * M_PI / n;
        for (int i = 0; i < n; i++) {
            x = cosf(a);
            y = sinf(a);
            
            glVertex3f(x * radius * 0.95f, y * radius * 0.95f, 0);
            glVertex3f(x * radius, y * radius, 0);
            
            a += da;
        }
    }
    
    glVertex3f(0, 0, -radius);
    glVertex3f(0, 0, radius);
    glVertex3f(-radius * vAxis.x, -radius * vAxis.y, -radius * vAxis.z);
    glVertex3f(radius * vAxis.x, radius * vAxis.y, radius * vAxis.z);
    
    glColor4f(1, 0, 0, 1);
    glVertex3f(radius, 0, 0);
    glVertex3f(radius + 10, 0, 0);
    glVertex3f(-radius, 0, 0);
    glVertex3f(-radius - 10, 0, 0);
    glColor4f(0, 1, 0, 1);
    glVertex3f(0, radius, 0);
    glVertex3f(0, radius + 10, 0);
    glVertex3f(0, -radius, 0);
    glVertex3f(0, -radius - 10, 0);
    glColor4f(0, 0, 1, 1);
    glVertex3f(0, 0, radius);
    glVertex3f(0, 0, radius + 10);
    glVertex3f(0, 0, -radius);
    glVertex3f(0, 0, -radius - 10);
    glEnd();

    glPopMatrix();
    glEnable(GL_CULL_FACE);
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

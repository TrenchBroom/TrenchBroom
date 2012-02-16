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

#import "ClipPlaneFeedbackFigure.h"

@implementation ClipPlaneFeedbackFigure

- (id)initWithPoint1:(const TVector3f *)thePoint1 point2:(const TVector3f *)thePoint2 point3:(const TVector3f *)thePoint3 {
    if ((self = [self init])) {
        point1 = *thePoint1;
        point2 = *thePoint2;
        point3 = *thePoint3;
    }
    
    return self;
}

- (void)render {
    glColor4f(0, 1, 0, 0.1f);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    glVertex3f(point1.x, point1.y, point1.z);
    glVertex3f(point2.x, point2.y, point2.z);
    glVertex3f(point3.x, point3.y, point3.z);
    glVertex3f(point3.x, point3.y, point3.z);
    glVertex3f(point2.x, point2.y, point2.z);
    glVertex3f(point1.x, point1.y, point1.z);
    glEnd();
    glEnable(GL_CULL_FACE);
}

@end

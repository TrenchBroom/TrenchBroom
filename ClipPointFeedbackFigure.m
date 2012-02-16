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

#import "ClipPointFeedbackFigure.h"
#import "Camera.h"

@implementation ClipPointFeedbackFigure

- (id)initWithPoint:(const TVector3f *)thePoint camera:(Camera *)theCamera {
    if (self = [self init]) {
        point = *thePoint;
        sphere = NULL;
        camera = theCamera;
    }
    
    return self;
}

- (void)render {
    if (sphere == NULL) {
        sphere = gluNewQuadric();
        gluQuadricDrawStyle(sphere, GLU_FILL);
    }
    
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT, GL_FILL);
    
    float dist = [camera distanceTo:&point];
    float radius = dist / 300 * 3;
    
    glPushMatrix();
    glTranslatef(point.x, point.y, point.z);
    glColor4f(0, 1, 0, 1);
    gluSphere(sphere, radius, 12, 12);
    glPopMatrix();
}

- (void)dealloc {
    if (sphere != NULL)
        gluDeleteQuadric(sphere);
    [super dealloc];
}
@end

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

#import "VertexFeedbackFigure.h"
#import "GLUtils.h"
#import "Camera.h"

@implementation VertexFeedbackFigure

- (id)initWithCamera:(Camera *)theCamera radius:(float)theRadius color:(const TVector4f *)theColor {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theRadius > 0, @"radius must be greater than 0");
    NSAssert(theColor != NULL, @"color must not be NULL");
    
    if ((self = [self init])) {
        camera = theCamera;
        radius = theRadius;
        color = *theColor;
        sphere = NULL;
    }
    
    return self;
}

- (void)dealloc {
    if (sphere != NULL)
        gluDeleteQuadric(sphere);
    [super dealloc];
}

- (void)render {
    if (sphere == NULL) {
        sphere = gluNewQuadric();
        gluQuadricDrawStyle(sphere, GLU_FILL);
    }
    
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT, GL_FILL);
    
    float dist = [camera distanceTo:&position];
    float actualRadius = dist / 300 * radius;
    
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glColorV4f(&color);
    gluSphere(sphere, actualRadius, 12, 12);
    glPopMatrix();
    
    glFrontFace(GL_CW);
}

- (void)setVertex:(const TVertex *)theVertex {
    NSAssert(theVertex != NULL, @"vertex must not be NULL");
    position = theVertex->position;
}

- (void)setPosition:(const TVector3f *)thePosition {
    NSAssert(thePosition != NULL, @"position must not be NULL");
    position = *thePosition;
}

@end

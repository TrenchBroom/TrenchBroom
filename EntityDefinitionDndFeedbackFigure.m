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

#import "EntityDefinitionDndFeedbackFigure.h"
#import "EntityDefinition.h"
#import "GLUtils.h"

@implementation EntityDefinitionDndFeedbackFigure

- (id)initWithEntityDefinition:(EntityDefinition *)theDefinition {
    NSAssert(theDefinition != nil, @"entity definition must not be nil");
    NSAssert([theDefinition type] == EDT_POINT, @"entity definition must be of type point");
    
    if ((self = [self init])) {
        definition = [theDefinition retain];
    }
    
    return self;
}

- (void)dealloc {
    [definition release];
    [super dealloc];
} 

- (void)setAxis:(EAxis)theAxis direction:(BOOL)theAxisDirection {
    axis = theAxis;
    axisDirection = theAxisDirection;
}

- (void)setOrigin:(TVector3i *)theOrigin {
    NSAssert(theOrigin != NULL, @"origin must not be NULL");
    origin = *theOrigin;
}

- (void)render:(id <Filter>)theFilter {
    TVector3f min, max;
    setV3f(&min, &origin);
    setV3f(&max, &origin);
    const TBoundingBox* bounds = [definition bounds];
    addV3f(&min, &bounds->min, &min);
    addV3f(&max, &bounds->max, &max);
    
    glDisable(GL_DEPTH_TEST);
    glColor4f(0, 1, 0, 1);
    glBegin(GL_LINE_LOOP);
    switch (axis) {
        case A_X:
            if (axisDirection) {
                glVertex3f(min.x, min.y, min.z);
                glVertex3f(min.x, max.y, min.z);
                glVertex3f(min.x, max.y, max.z);
                glVertex3f(min.x, min.y, max.z);
            } else {
                glVertex3f(max.x, min.y, min.z);
                glVertex3f(max.x, max.y, min.z);
                glVertex3f(max.x, max.y, max.z);
                glVertex3f(max.x, min.y, max.z);
            }
            break;
        case A_Y:
            if (axisDirection) {
                glVertex3f(min.x, min.y, min.z);
                glVertex3f(max.x, min.y, min.z);
                glVertex3f(max.x, min.y, max.z);
                glVertex3f(min.x, min.y, max.z);
            } else {
                glVertex3f(min.x, max.y, min.z);
                glVertex3f(max.x, max.y, min.z);
                glVertex3f(max.x, max.y, max.z);
                glVertex3f(min.x, max.y, max.z);
            }
            break;
        case A_Z:
            if (axisDirection) {
                glVertex3f(min.x, min.y, min.z);
                glVertex3f(max.x, min.y, min.z);
                glVertex3f(max.x, max.y, min.z);
                glVertex3f(min.x, max.y, min.z);
            } else {
                glVertex3f(min.x, min.y, max.z);
                glVertex3f(max.x, min.y, max.z);
                glVertex3f(max.x, max.y, max.z);
                glVertex3f(min.x, max.y, max.z);
            }
            break;
    }
    glEnd();
}

@end

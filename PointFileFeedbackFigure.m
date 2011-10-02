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

#import "PointFileFeedbackFigure.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "GLUtils.h"

@implementation PointFileFeedbackFigure

- (id)initWithPoints:(TVector3f *)thePoints pointCount:(int)thePointCount {
    NSAssert(thePoints != NULL, @"point list must not be NULL");
    NSAssert(thePointCount > 0, @"point count must be greater than 0");
    
    if ((self = [self init])) {
        pointCount = thePointCount;
        points = malloc(pointCount * sizeof(TVector3f));
        memcpy(points, thePoints, pointCount * sizeof(TVector3f));
    }
    
    return self;
}

- (void)dealloc {
    if (points != NULL)
        free(points);
    [super dealloc];
}

- (void)render {
    glDisable(GL_DEPTH_TEST);
    glColor4f(1, 1, 0, 0.3f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < pointCount; i++)
        glVertexV3f(&points[i]);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    
    glColor4f(1, 1, 0, 1);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < pointCount; i++)
        glVertexV3f(&points[i]);
    glEnd();
}

@end

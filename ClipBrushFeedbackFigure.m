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

#import "ClipBrushFeedbackFigure.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "ClipPlane.h"
#import "VertexData.h"
#import "GLUtils.h"

@implementation ClipBrushFeedbackFigure

- (id)initWithBrush:(id <Brush>)theBrush clipPlane:(ClipPlane *)theClipPlane {
    NSAssert(theBrush != nil, @"brush must not be nil");
    NSAssert(theClipPlane != nil, @"clip plane must not be nil");
    
    if (self = [self init]) {
        [theClipPlane clipBrush:theBrush firstResult:&brush1 secondResult:&brush2];
        if (brush1 == nil && brush2 == nil) {
            [self release];
            return nil;
        } else {
            if (brush1 != nil)
                [brush1 retain];
            if (brush2 != nil)
                [brush2 retain];
        }
    }
    
    return self;
}

- (void)render {
    if (brush1 != nil || brush2 != nil) {
        glSetEdgeOffset(0.6f);
        glEnable(GL_DEPTH_TEST);
        glColor4f(0, 1, 0, 1);
        glBegin(GL_LINES);
        if (brush1 != nil) {
            TEdge** edges = [brush1 edges];
            for (int i = 0; i < [brush1 edgeCount]; i++) {
                glVertexV3f(&edges[i]->startVertex->vector);
                glVertexV3f(&edges[i]->endVertex->vector);
            }
        }
        if (brush2 != nil) {
            TEdge** edges = [brush2 edges];
            for (int i = 0; i < [brush2 edgeCount]; i++) {
                glVertexV3f(&edges[i]->startVertex->vector);
                glVertexV3f(&edges[i]->endVertex->vector);
            }
        }
        glEnd();
        glDisable(GL_DEPTH_TEST);
        glResetEdgeOffset();
    }
}

- (void)dealloc {
    [brush1 release];
    [brush2 release];
    [super dealloc];
}
@end

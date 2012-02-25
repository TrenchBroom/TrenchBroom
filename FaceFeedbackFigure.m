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

#import "FaceFeedbackFigure.h"
#import "VertexFeedbackFigure.h"
#import "GLUtils.h"

@implementation FaceFeedbackFigure

- (id)initWithCamera:(Camera *)theCamera radius:(float)theRadius color:(const TVector4f *)theColor {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theRadius > 0, @"radius must be greater than 0");
    NSAssert(theColor != NULL, @"color must not be NULL");
    
    if ((self = [self init])) {
        color = *theColor;
        vertexFigure = [[VertexFeedbackFigure alloc] initWithCamera:theCamera radius:theRadius color:theColor];
    }
    
    return self;
}

- (void)dealloc {
    [vertexFigure release];
    [super dealloc];
}

- (void)render {
    TVertex* current;
    TVertex* next;
    TVector3f center;
    
    glColorV4f(&color);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < vertices->count; i++)
        glVertexV3f(&vertices->items[i]->position);
    glEnd();
    
    for (int i = 0; i < vertices->count; i++) {
        current = vertices->items[i];
        next = vertices->items[(i + 1) % vertices->count];
        
        [vertexFigure setVertex:current];
        [vertexFigure render];
        
        addV3f(&current->position, &next->position, &center);
        scaleV3f(&center, 0.5f, &center);
        
        [vertexFigure setPosition:&center];
        [vertexFigure render];
    }
    
    centerOfVertices(vertices, &center);
    [vertexFigure setPosition:&center];
    [vertexFigure render];

}

- (void)setVertices:(const TVertexList *)theVertices {
    NSAssert(theVertices != NULL, @"vertices must not be NULL");
    vertices = theVertices;
}

@end

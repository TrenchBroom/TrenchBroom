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

#import "EdgeFeedbackFigure.h"
#import "VertexFeedbackFigure.h"
#import "GLUtils.h"

@implementation EdgeFeedbackFigure

- (id)initWithCamera:(Camera *)theCamera radius:(float)theRadius color:(const TVector4f *)theColor {
    NSAssert(theCamera != nil, @"camera must not be nil");
    NSAssert(theRadius > 0, @"radius must be greater than 0");
    NSAssert(theColor != NULL, @"color must not be NULL");
    
    if ((self = [self init])) {
        startFigure = [[VertexFeedbackFigure alloc] initWithCamera:theCamera radius:theRadius color:theColor];
        endFigure = [[VertexFeedbackFigure alloc] initWithCamera:theCamera radius:theRadius color:theColor];
        centerFigure = [[VertexFeedbackFigure alloc] initWithCamera:theCamera radius:theRadius color:theColor];
        color = *theColor;
    }
    
    return self;
}

- (void)dealloc {
    [startFigure release];
    [endFigure release];
    [centerFigure release];
    [super dealloc];
}

- (void)render:(id <Filter>)theFilter {
    glColorV4f(&color);
    glBegin(GL_LINES);
    glVertexV3f(&start);
    glVertexV3f(&end);
    glEnd();
    
    [startFigure render:theFilter];
    [endFigure render:theFilter];
    [centerFigure render:theFilter];
}

- (void)setEdge:(const TEdge *)theEdge {
    NSAssert(theEdge != NULL, @"edge must not be NULL");

    TVector3f center;
    
    start = theEdge->startVertex->position;
    end = theEdge->endVertex->position;

    [startFigure setVertex:theEdge->startVertex];
    [endFigure setVertex:theEdge->endVertex];
    
    addV3f(&start, &end, &center);
    scaleV3f(&center, 0.5f, &center);
    [centerFigure setPosition:&center];
}

@end

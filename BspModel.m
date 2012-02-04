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

#import "BspModel.h"
#import "BspFace.h"

@implementation BspModel

- (id)initWithFaces:(NSArray *)theFaces vertexCount:(int)theVertexCount center:(TVector3f *)theCenter bounds:(TBoundingBox *)theBounds maxBounds:(TBoundingBox *)theMaxBounds {
    NSAssert(theFaces != nil, @"face list must not be nil");
    NSAssert([theFaces count] >= 4, @"face list must contain at least 4 faces");
    NSAssert(theVertexCount >= 12, @"model must have at least 12 vertices");
    NSAssert(theCenter != NULL, @"center must not be NULL");
    NSAssert(theBounds != NULL, @"bounds must not be NULL");
    NSAssert(theMaxBounds != NULL, @"max bounds must not be NULL");
    
    if ((self = [self init])) {
        faces = [theFaces retain];
        vertexCount = theVertexCount;
        center = *theCenter;
        bounds = *theBounds;
        maxBounds = *theMaxBounds;
    }
    
    return self;
}

- (void)dealloc {
    [faces release];
    [super dealloc];
}

- (const TVector3f *)center {
    return &center;
}

- (const TBoundingBox *)bounds {
    return &bounds;
}

- (const TBoundingBox *)maxBounds {
    return &maxBounds;
}

- (NSArray *)faces {
    return faces;
}

- (int)vertexCount {
    return vertexCount;
}

@end

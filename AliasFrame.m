/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "AliasFrame.h"

@implementation AliasFrame

- (id)initWithName:(NSString *)theName triangles:(TFrameTriangle *)theTriangles triangleCount:(int)theTriangleCount center:(TVector3f *)theCenter bounds:(TBoundingBox *)theBounds maxBounds:(TBoundingBox *)theMaxBounds {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theTriangles != NULL, @"triangle array must not be NULL");
    NSAssert(theTriangleCount > 0, @"triangle count must be greater than 0");
    NSAssert(theCenter != NULL, @"center must not be NULL");
    NSAssert(theBounds != NULL, @"bounds must not be NULL");
    NSAssert(theMaxBounds != NULL, @"max bounds must not be NULL");
    
    if ((self = [self init])) {
        name = [[NSString alloc] initWithString:theName];
        triangleCount = theTriangleCount;
        triangles = malloc(triangleCount * sizeof(TFrameTriangle));
        memcpy(triangles, theTriangles, triangleCount * sizeof(TFrameTriangle));
        center = *theCenter;
        bounds = *theBounds;
        maxBounds = *theMaxBounds;
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    free(triangles);
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (int)triangleCount {
    return triangleCount;
}

- (const TFrameTriangle *)triangleAtIndex:(int)theIndex {
    NSAssert(theIndex >= 0 && theIndex < triangleCount, @"index out of bounds");
    return &triangles[theIndex];
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

@end

//
//  EntityModelFrame.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Alias.h"
#import "Math.h"

@interface AliasFrame : NSObject {
    NSString* name;
    TFrameTriangle* triangles;
    int triangleCount;
    TVector3f center;
    TBoundingBox bounds;
    TBoundingBox maxBounds;
}

- (id)initWithName:(NSString *)theName triangles:(TFrameTriangle *)theTriangles triangleCount:(int)theTriangleCount center:(TVector3f *)theCenter bounds:(TBoundingBox *)theBounds maxBounds:(TBoundingBox *)theMaxBounds;

- (NSString *)name;
- (int)triangleCount;
- (const TFrameTriangle *)triangleAtIndex:(int)theIndex;
- (const TVector3f *)center;
- (const TBoundingBox *)bounds;
- (const TBoundingBox *)maxBounds;

@end

//
//  VertexData2.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MutableFace;
@class BoundingBox;
@class Ray3D;
@class PickingHitList;
@class Vector3f;

@interface VertexData : NSObject {
    @private
    NSMutableArray* vertices;
    NSMutableArray* edges;
    NSMutableArray* sides;
    BoundingBox* bounds;
    BoundingBox* pickingBounds;
    Vector3f* center;
}

- (id)initWithFaces:(NSArray *)faces droppedFaces:(NSMutableSet **)droppedFaces;

- (BOOL)cutWithFace:(MutableFace *)face droppedFaces:(NSMutableSet **)droppedFaces;
- (NSArray *)vertices;
- (NSArray *)edges;
- (int)edgeCount;
- (BoundingBox *)bounds;
- (Vector3f *)center;

- (void)pick:(Ray3D *)theRay hitList:(PickingHitList *)theHitList;
- (BoundingBox *)pickingBounds;

@end

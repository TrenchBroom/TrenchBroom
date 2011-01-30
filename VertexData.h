//
//  VertexData2.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    BM_KEEP,
    BM_DROP,
    BM_SPLIT
} EBrushMark;

@class Face;
@class BoundingBox;
@class Ray3D;
@class PickingHit;

@interface VertexData : NSObject {
    @private
    NSMutableArray* vertices;
    NSMutableArray* edges;
    NSMutableArray* sides;
    NSMutableArray* sideToFace;
    NSMutableDictionary* faceToSide;
    BoundingBox* bounds;
}

- (BOOL)cutWithFace:(Face *)face droppedFaces:(NSMutableArray **)droppedFaces;
- (NSArray*)verticesForFace:(Face *)face;
- (BoundingBox *)bounds;
- (PickingHit *)pickFace:(Ray3D *)theRay;
@end

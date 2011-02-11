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
@class Vector3f;

@interface VertexData : NSObject {
    @private
    NSMutableArray* vertices;
    NSMutableArray* edges;
    NSMutableArray* sides;
    NSMutableDictionary* faceToSide;
    NSMutableDictionary* centers;
    BoundingBox* bounds;
}

- (id)initWithFaces:(NSArray *)faces droppedFaces:(NSMutableArray **)droppedFaces;

- (BOOL)cutWithFace:(Face *)face droppedFaces:(NSMutableArray **)droppedFaces;
- (NSArray *)verticesForFace:(Face *)face;
- (NSArray *)verticesForWireframe;
- (int)edgeCount;
- (BoundingBox *)bounds;

/*!
    @function
    @abstract   Returns the center of the given face.
    @param      The face.
    @result     The center of the given face.
*/
- (Vector3f *)centerOfFace:(Face *)face;

- (PickingHit *)pickFace:(Face *)theFace withRay:(Ray3D *)theRay;
@end

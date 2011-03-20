//
//  Side.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "VertexData.h"

typedef enum {
    SM_KEEP,
    SM_DROP,
    SM_SPLIT,
    SM_NEW,
    SM_UNKNOWN
} ESideMark;

@class MutableFace;
@class Edge;
@class SideEdge;
@class Vector3f;
@class Ray3D;
@class PickingHit;

@interface Side : NSObject {
    @private
    MutableFace * face;
    NSMutableArray* sideEdges;
    ESideMark mark;
    NSMutableArray* vertices;
    NSMutableArray* edges;
    Vector3f* center;
}

- (id)initWithFace:(MutableFace *)theFace edges:(NSArray *)theEdges flipped:(BOOL*)flipped;
- (id)initWithFace:(MutableFace *)theFace sideEdges:(NSArray *)theEdges;

- (SideEdge *)split;

- (ESideMark)mark;
- (void)setMark:(ESideMark)theMark;

- (NSArray *)vertices;
- (NSArray *)edges;
- (MutableFace *)face;
- (PickingHit *)pickWithRay:(Ray3D *)theRay;

/*!
    @function
    @abstract   Returns the center of this side.
    @result     The center of this side.
*/
- (Vector3f *)center;
@end

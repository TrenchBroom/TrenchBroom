//
//  Vertex.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    VM_DROP,
    VM_KEEP,
    VM_UNDECIDED,
    VM_NEW,
    VM_UNKNOWN
} EVertexMark;

@class Edge;
@class Vector3f;
@class Vector3i;
@class PickingHit;
@class Ray3D;

@interface Vertex : NSObject {
    @private
    Vector3f* vector;
    NSMutableSet* edges;
    EVertexMark mark;
}

- (id)initWithVector:(Vector3f *)theVector;

- (Vector3f *)vector;
- (void)addEdge:(Edge *)theEdge;
- (NSSet *)edges;

- (EVertexMark)mark;
- (void)setMark:(EVertexMark)theMark;

- (PickingHit *)pickWithRay:(Ray3D *)theRay;
@end

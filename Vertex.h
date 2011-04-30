//
//  Vertex.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    VM_DROP,
    VM_KEEP,
    VM_UNDECIDED,
    VM_NEW,
    VM_UNKNOWN
} EVertexMark;

@class Edge;
@class PickingHit;

@interface Vertex : NSObject {
    @private
    TVector3f vector;
    NSMutableSet* edges;
    EVertexMark mark;
}

- (id)initWithVector:(TVector3f *)theVector;
- (id)initWithX:(float)x y:(float)y z:(float)z;

- (TVector3f *)vector;
- (void)addEdge:(Edge *)theEdge;
- (NSSet *)edges;

- (EVertexMark)mark;
- (void)setMark:(EVertexMark)theMark;
@end

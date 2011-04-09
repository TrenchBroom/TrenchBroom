//
//  Edge.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    EM_KEEP,
    EM_DROP,
    EM_SPLIT,
    EM_UNDECIDED,
    EM_NEW,
    EM_UNKNOWN
} EEdgeMark;

@class Vertex;
@class Side;
@class Vector3f;
@class Plane3D;
@class Ray3D;
@class PickingHit;
@class BoundingBox;
@class VBOMemBlock;
@protocol Face;

@interface Edge : NSObject {
    @private
    Vertex* startVertex;
    Vertex* endVertex;
    Side* leftSide;
    Side* rightSide;
    EEdgeMark mark;
}

- (id)initWithStartVertex:(Vertex *)theStartVertex endVertex:(Vertex *)theEndVertex;

- (Vertex *)startVertex;
- (Vertex *)endVertex;

- (id <Face>)leftFace;
- (id <Face>)rightFace;
- (Side *)leftSide;
- (Side *)rightSide;
- (Vertex *)startVertexForSide:(Side *)theSide;
- (Vertex *)endVertexForSide:(Side *)theSide;
- (void)setLeftSide:(Side *)theLeftSide;
- (void)setRightSide:(Side *)theRightSide;
- (void)flip;

- (Vertex *)splitAt:(Plane3D *)plane;

- (PickingHit *)pickWithRay:(Ray3D *)theRay;
- (void)expandBounds:(BoundingBox *)theBounds;

- (EEdgeMark)mark;
- (void)updateMark;
- (void)clearMark;

@end

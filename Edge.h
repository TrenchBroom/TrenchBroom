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
    EM_NEW,
    EM_UNKNOWN
} EEdgeMark;

@class Vertex;
@class SideEdge;
@class Vector3f;
@class Plane3D;
@class Ray3D;
@class PickingHit;
@class BoundingBox;
@protocol Face;

@interface Edge : NSObject {
    @private
    Vertex* startVertex;
    Vertex* endVertex;
    SideEdge* leftEdge;
    SideEdge* rightEdge;
    EEdgeMark mark;
}
- (id)initWithStartVertex:(Vertex *)theStartVertex endVertex:(Vertex *)theEndVertex;

- (Vertex *)startVertex;
- (Vertex *)endVertex;

- (id <Face>)leftFace;
- (id <Face>)rightFace;
- (void)setLeftEdge:(SideEdge *)theLeftEdge;
- (void)setRightEdge:(SideEdge *)theRightEdge;

- (Vertex *)splitAt:(Plane3D *)plane;

- (PickingHit *)pickWithRay:(Ray3D *)theRay;
- (void)expandBounds:(BoundingBox *)theBounds;

- (EEdgeMark)mark;
- (void)updateMark;

@end

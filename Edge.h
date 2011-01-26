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
@class Vector3f;
@class Plane3D;

@interface Edge : NSObject {
    @private
    Vertex* startVertex;
    Vertex* endVertex;
    EEdgeMark mark;
}
- (id)initWithStartVertex:(Vertex *)theStartVertex endVertex:(Vertex *)theEndVertex;

- (Vertex *)startVertex;
- (Vertex *)endVertex;

- (Vertex *)splitAt:(Plane3D *)plane;

- (EEdgeMark)mark;
- (void)updateMark;

@end

//
//  SideEdge.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Edge.h"

@class Vertex;
@class Side;

@interface SideEdge : NSObject {
    @private
    Side* side;
    Edge* edge;
    BOOL flipped;
}

- (id)initWithEdge:(Edge *)theEdge flipped:(BOOL)isFlipped;

- (Vertex *)startVertex;
- (Vertex *)endVertex;
- (Side *)side;
- (void)setSide:(Side *)theSide;
- (EEdgeMark)mark;
- (Edge *)edge;

@end

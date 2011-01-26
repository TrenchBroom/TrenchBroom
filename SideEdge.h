//
//  SideEdge.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Edge.h"

@interface SideEdge : NSObject {
    Edge* edge;
    BOOL flipped;
}

- (id)initWithEdge:(Edge *)theEdge flipped:(BOOL)isFlipped;

- (Vertex *)startVertex;
- (Vertex *)endVertex;
- (EEdgeMark)mark;
- (Edge *)edge;

@end

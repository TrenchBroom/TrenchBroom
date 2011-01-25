//
//  Side.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "VertexData.h"

@class Edge;

typedef struct {
    EObjectStatus status;
    Edge* newEdge;
} ESideUpdateResult;

@interface Side : NSObject {
    int* edgeIndices;
    BOOL* flipped;
    int edgeCount;
}

- (id)initWithEdges:(int*)theEdgeIndices flipped:(BOOL*)theFlipped count:(int)theCount;

- (ESideUpdateResult)updateEdges:(EObjectStatus *)edgeStatus newEdgeIndex:(int)newEdgeIndex edges:(NSArray *)edges;

- (NSArray *)verticesWidthEdgeArray:(NSArray *)edges vertexArray:(NSArray *)vertices;
@end

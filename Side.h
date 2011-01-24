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

@interface Side : NSObject {
    int* edgeIndices;
    BOOL* flipped;
    int edgeCount;
}

- (id)initWithEdges:(int*)theEdgeIndices flipped:(BOOL*)theFlipped count:(int)theCount;

- (BOOL)updateEdges:(EObjectStatus *)edgeStatus edges:(NSMutableArray *)edges;

@end

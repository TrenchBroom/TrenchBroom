//
//  Side.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Side.h"
#import "Edge.h"

@implementation Side
- (id)initWithEdges:(int*)theEdgeIndices flipped:(BOOL*)theFlipped count:(int)theCount {
    if (self = [self init]) {
        edgeIndices = theEdgeIndices;
        flipped = theFlipped;
        edgeCount = theCount;
    }
    
    return self;
}

- (int)startVertexIndex:(int)edgeIndex edges:(NSArray *)edges {
    if (flipped[edgeIndex])
        return [[edges objectAtIndex:edgeIndex] endIndex];
    return [[edges objectAtIndex:edgeIndex] startIndex];
}

- (int)endVertexIndex:(int)edgeIndex edges:(NSArray *)edges {
    if (flipped[edgeIndex])
        return [[edges objectAtIndex:edgeIndex] startIndex];
    return [[edges objectAtIndex:edgeIndex] endIndex];
}

- (BOOL)updateEdges:(EObjectStatus *)edgeStatus edges:(NSMutableArray *)edges {
    EObjectStatus previousStatus = edgeStatus[edgeIndices[edgeCount - 1]];
    EObjectStatus sideStatus = previousStatus;

    int splitIndex1, splitIndex2;
    for (int i = 0; i < edgeCount; i++) {
        EObjectStatus currentStatus = edgeStatus[edgeIndices[i]];
        if (currentStatus == OS_SPLIT) {
            if (previousStatus == OS_KEEP) {
                splitIndex1 = i;
            } else {
                splitIndex2 = i;
            }
        }
        
        if (sideStatus != currentStatus)
            sideStatus = OS_SPLIT;
        previousStatus = currentStatus;
    }
    
    if (sideStatus == OS_KEEP)
        return YES;
    if (sideStatus == OS_DROP)
        return NO;

    if (splitIndex2 > splitIndex1) {
        int newEdgeCount = edgeCount - (splitIndex2 - splitIndex1 - 1);
        int newEdgeIndices[newEdgeCount];
        BOOL newFlipped[newEdgeCount];

        for (int i = 0; i <= splitIndex1; i++) {
            newEdgeIndices[i + edgeCount - splitIndex2] = edgeIndices[i];
            newFlipped[i + edgeCount - splitIndex2] = flipped[i];
        }
        for (int i = splitIndex2; i < edgeCount; i++) {
            newEdgeIndices[i - splitIndex2] = edgeIndices[i];
            newFlipped[i - splitIndex2] = newFlipped[i];
        }
            
        free(edgeIndices);
        free(flipped);
        edgeIndices = newEdgeIndices;
        flipped = newFlipped;
        edgeCount = newEdgeCount;

        int startVertexIndex = [self startVertexIndex:edgeCount - 2 edges:edges];
        int endVertexIndex = [self endVertexIndex:0 edges:edges];
        Edge* newEdge = [[Edge alloc] initWithStartIndex:startVertexIndex endIndex:endVertexIndex];
        [edges addObject:newEdge];
        [newEdge release];
        edgeIndices[edgeCount - 1] = [edges count] - 1;
        flipped[edgeCount - 1] = NO; 
    } else {
        int newEdgeCount = splitIndex1 - splitIndex2 + 2;
        int newEdgeIndices[newEdgeCount];
        BOOL newFlipped[newEdgeCount];
        
        for (int i = splitIndex2; i <= splitIndex1; i++) {
            newEdgeIndices[i - splitIndex2] = edgeIndices[i];
            newFlipped[i - splitIndex2] = flipped[i];
        }
        
        free(edgeIndices);
        free(flipped);
        edgeIndices = newEdgeIndices;
        flipped = newFlipped;
        edgeCount = newEdgeCount;

        int startVertexIndex = [self startVertexIndex:0 edges:edges];
        int endVertexIndex = [self endVertexIndex:edgeCount - 2 edges:edges];
        Edge* newEdge = [[Edge alloc] initWithStartIndex:startVertexIndex endIndex:endVertexIndex];
        [edges addObject:newEdge];
        [newEdge release];
        edgeIndices[edgeCount - 1] = [edges count] - 1;
        flipped[edgeCount - 1] = NO; 
    }

    return YES;
}

- (void)dealloc {
    free(edgeIndices);
    free(flipped);
    [super dealloc];
}

@end

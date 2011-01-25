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
        edgeCount = theCount;
        edgeIndices = malloc(edgeCount * sizeof(int));
        memcpy(edgeIndices, theEdgeIndices, edgeCount * sizeof(int));
        flipped = malloc(edgeCount * sizeof(BOOL));
        memcpy(flipped, theFlipped, edgeCount * sizeof(BOOL));
    }
    
    return self;
}

- (int)startVertexIndex:(int)edgeIndex edges:(NSArray *)edges {
    if (flipped[edgeIndex])
        return [[edges objectAtIndex:edgeIndices[edgeIndex]] endIndex];
    return [[edges objectAtIndex:edgeIndices[edgeIndex]] startIndex];
}

- (int)endVertexIndex:(int)edgeIndex edges:(NSArray *)edges {
    if (flipped[edgeIndex])
        return [[edges objectAtIndex:edgeIndices[edgeIndex]] startIndex];
    return [[edges objectAtIndex:edgeIndices[edgeIndex]] endIndex];
}

- (ESideUpdateResult)updateEdges:(EObjectStatus *)edgeStatus newEdgeIndex:(int)newEdgeIndex edges:(NSArray *)edges {
    EObjectStatus previousStatus = edgeStatus[edgeIndices[edgeCount - 1]];
    EObjectStatus sideStatus = previousStatus;

    // determine the side status and the indices of the split edges, if any
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
    
    ESideUpdateResult result;
    result.status = sideStatus;
    result.newEdge = nil;
 
    if (result.status == OS_SPLIT) {
        // split the side by rotating the edge index array such that the newly created edge can be appended at the end of the array
        // then, create the new edge and return it
        if (splitIndex2 > splitIndex1) {
            int newEdgeCount = edgeCount - (splitIndex2 - splitIndex1) + 2;
            int* newEdgeIndices = malloc(newEdgeCount * sizeof(int));
            BOOL* newFlipped = malloc(newEdgeCount * sizeof(BOOL));
            
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
            
            int startVertexIndex = [self endVertexIndex:edgeCount - 2 edges:edges];
            int endVertexIndex = [self startVertexIndex:0 edges:edges];
            result.newEdge = [[Edge alloc] initWithStartIndex:startVertexIndex endIndex:endVertexIndex];
            edgeIndices[edgeCount - 1] = newEdgeIndex;
            flipped[edgeCount - 1] = NO; 
        } else {
            int newEdgeCount = splitIndex1 - splitIndex2 + 2;
            int* newEdgeIndices = malloc(newEdgeCount * sizeof(int));
            BOOL* newFlipped = malloc(newEdgeCount * sizeof(BOOL));
            
            for (int i = splitIndex2; i <= splitIndex1; i++) {
                newEdgeIndices[i - splitIndex2] = edgeIndices[i];
                newFlipped[i - splitIndex2] = flipped[i];
            }
            
            free(edgeIndices);
            free(flipped);
            edgeIndices = newEdgeIndices;
            flipped = newFlipped;
            edgeCount = newEdgeCount;
            
            int startVertexIndex = [self endVertexIndex:0 edges:edges];
            int endVertexIndex = [self startVertexIndex:edgeCount - 2 edges:edges];
            result.newEdge = [[Edge alloc] initWithStartIndex:startVertexIndex endIndex:endVertexIndex];
            edgeIndices[edgeCount - 1] = newEdgeIndex;
            flipped[edgeCount - 1] = NO; 
        }
    }
    
    return result;
}

- (NSArray *)verticesWidthEdgeArray:(NSArray *)edges vertexArray:(NSArray *)vertices {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    for (int i = 0; i < edgeCount; i++) {
        Edge* edge = [edges objectAtIndex:edgeIndices[i]];
        if (flipped[i])
            [result addObject:[vertices objectAtIndex:[edge endIndex]]];
        else
            [result addObject:[vertices objectAtIndex:[edge startIndex]]];
    }
    
    return [result autorelease];
}

- (NSString *)description {
    NSMutableString* desc = [[NSMutableString alloc] init];
    [desc appendFormat:@"%i edges, edges: ", edgeCount];
    for (int i = 0; i < edgeCount; i++) {
        [desc appendFormat:@"%i", edgeIndices[i]];
        if (flipped[i])
            [desc appendFormat:@" (flipped)"];
        if (i < edgeCount - 1)
            [desc appendFormat:@", "];
    }
    
    return desc;
}

- (void)dealloc {
    free(edgeIndices);
    free(flipped);
    [super dealloc];
}

@end

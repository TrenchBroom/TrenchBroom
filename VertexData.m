//
//  VertexData.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "VertexData.h"
#import "Vector3f.h"
#import "Edge.h"
#import "Side.h"
#import "HalfSpace3D.h"
#import "Line3D.h"
#import "Plane3D.h"
#import "Face.h"

@implementation VertexData
- (id)init {
    if (self = [super init]) {
        vertices = [[NSMutableArray alloc] init];
        edges = [[NSMutableArray alloc] init];
        sides = [[NSMutableArray alloc] init];
        faceToSide = [[NSMutableDictionary alloc] init];

        // initialize as huge cube
        Vector3f* v = [[Vector3f alloc] initWithX:-4096 y:-4096 z:-4096];
        [vertices addObject:v];
        [v release];
        int lbb = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:-4096 y:-4096 z:+4096];
        [vertices addObject:v];
        [v release];
        int lbt = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:-4096 y:+4096 z:-4096];
        [vertices addObject:v];
        [v release];
        int lfb = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:-4096 y:+4096 z:+4096];
        [vertices addObject:v];
        [v release];
        int lft = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:+4096 y:-4096 z:-4096];
        [vertices addObject:v];
        [v release];
        int rbb = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:+4096 y:-4096 z:+4096];
        [vertices addObject:v];
        [v release];
        int rbt = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:+4096 y:+4096 z:-4096];
        [vertices addObject:v];
        [v release];
        int rfb = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:+4096 y:+4096 z:+4096];
        [vertices addObject:v];
        [v release];
        int rft = [vertices count] - 1;

        // create edges
        Edge* e = [[Edge alloc] initWithStartIndex:lbb endIndex:lbt];
        [edges addObject:e];
        [e release];
        int lbblbt = [edges count] - 1;

        e = [[Edge alloc] initWithStartIndex:lbb endIndex:lfb];
        [edges addObject:e];
        [e release];
        int lbblfb = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:lbb endIndex:rbb];
        [edges addObject:e];
        [e release];
        int lbbrbb = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:rft endIndex:rfb];
        [edges addObject:e];
        [e release];
        int rftrfb = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:rft endIndex:rbt];
        [edges addObject:e];
        [e release];
        int rftrbt = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:rft endIndex:lft];
        [edges addObject:e];
        [e release];
        int rftlft = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:lbt endIndex:lft];
        [edges addObject:e];
        [e release];
        int lbtlft = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:lbt endIndex:rbt];
        [edges addObject:e];
        [e release];
        int lbtrbt = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:lfb endIndex:lft];
        [edges addObject:e];
        [e release];
        int lfblft = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:lfb endIndex:rfb];
        [edges addObject:e];
        [e release];
        int lfbrfb = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:rbb endIndex:rbt];
        [edges addObject:e];
        [e release];
        int rbbrbt = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:rbb endIndex:rfb];
        [edges addObject:e];
        [e release];
        int rbbrfb = [edges count] - 1;
        
        // create sides
    }
    
    return self;
}

- (BOOL)cutWithFace:(Face *)face droppedFaces:(NSArray **)droppedFaces {
    HalfSpace3D* halfSpace = [face halfSpace];
    
    int oldVertexCount = [vertices count];
    int oldEdgeCount = [edges count];
    
    BOOL vertexStatus[[vertices count]];
    vertexStatus[0] = [halfSpace containsPoint:[vertices objectAtIndex:0]];
    EObjectStatus status = vertexStatus[0] ? OS_KEEP : OS_DROP;
    
    for (int i = i; i < [vertices count]; i++) {
        Vector3f* vertex = [vertices objectAtIndex:i];
        vertexStatus[i] = [halfSpace containsPoint:vertex];
        if (vertexStatus[i - 1] != vertexStatus[i])
            status = OS_SPLIT;
    }

    if (status == OS_KEEP)
        return YES;
    
    if (status == OS_DROP)
        return NO;
                            
    Line3D* line = [[Line3D alloc] init];
    EObjectStatus edgeStatus[[edges count]];
    for (int i = 0; i < [edges count]; i++) {
        Edge* edge = [edges objectAtIndex:i];
        BOOL startStatus = vertexStatus[[edge startIndex]];
        BOOL endStatus = vertexStatus[[edge endIndex]];
        
        if (startStatus && endStatus) {
            edgeStatus[i] = OS_KEEP;
        } else if (!startStatus && !endStatus) {
            edgeStatus[i] = OS_DROP;
        } else {
            edgeStatus[i] = OS_SPLIT;
            [line setPoint1:[edge startVertex:vertices] point2:[edge endVertex:vertices]];
            Vector3f* newVertex = [[halfSpace boundary] intersectWithLine:line];
            [vertices addObject:newVertex];
            if (startStatus == OS_DROP)
                [edge setStartIndex:[vertices count] - 1];
            else
                [edge setEndIndex:[vertices count] - 1];
        }
    }
    [line release];
    
    for (int i = 0; i < [sides count]; i++) {
        Side* side = [sides objectAtIndex:i];
        if (![side updateEdges:edgeStatus edges:edges])
            [sides removeObjectAtIndex:i--];
    }

    // create a new side from the newly created edges
    int newSideEdgeCount = [edges count] - oldEdgeCount;
    int newSideEdges[newSideEdgeCount];
    BOOL newSideFlipped[newSideEdgeCount];
    
    newSideFlipped[0] = NO;
    for (int i = 0; i < newSideEdgeCount; i++)
        newSideEdges[i] = i + oldEdgeCount;

    for (int i = 0; i < newSideEdgeCount - 1; i++) {
        Edge* edge = [edges objectAtIndex:newSideEdges[i]];
        int edgeEndIndex = newSideFlipped[i] ? [edge startIndex] : [edge endIndex];
        for (int j = i + 1; j < newSideEdgeCount; j++) {
            Edge* candidate = [edges objectAtIndex:newSideEdges[j]];
            if (edgeEndIndex == [candidate startIndex] || edgeEndIndex == [candidate endIndex]) {
                if (j > i + 1) {
                    int t = newSideEdges[i + 1];
                    newSideEdges[i + 1] = newSideEdges[j];
                    newSideEdges[j] = t;
                }
                newSideFlipped[i + 1] = edgeEndIndex == [candidate endIndex];
            }
        }
    }

    Side* side = [[Side alloc] initWithEdges:newSideEdgeIndices flipped:newSideFlipped count:newEdgeCount];
    [sides addObject:side];
    [faceToSide setObject:side forKey:[face getId]];

    
    
    return YES;
}

- (void)dealloc {
    [faceToSide release];
    [sides release];
    [edges release];
    [vertices release];
    [super dealloc];
}

@end

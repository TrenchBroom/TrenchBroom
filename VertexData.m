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
        sideToFace = [[NSMutableArray alloc] init];
        freeVertexSlots = [[NSMutableArray alloc] init];
        freeEdgeSlots = [[NSMutableArray alloc] init];
        freeSideSlots = [[NSMutableArray alloc] init];
        null = [[NSObject alloc] init];

        // initialize as huge cube
        Vector3f* v = [[Vector3f alloc] initWithX:-4096 y:-4096 z:-4096];
        [vertices addObject:v];
        [v release];
        int wsb = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:-4096 y:-4096 z:+4096];
        [vertices addObject:v];
        [v release];
        int wst = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:-4096 y:+4096 z:-4096];
        [vertices addObject:v];
        [v release];
        int wnb = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:-4096 y:+4096 z:+4096];
        [vertices addObject:v];
        [v release];
        int wnt = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:+4096 y:-4096 z:-4096];
        [vertices addObject:v];
        [v release];
        int esb = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:+4096 y:-4096 z:+4096];
        [vertices addObject:v];
        [v release];
        int est = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:+4096 y:+4096 z:-4096];
        [vertices addObject:v];
        [v release];
        int enb = [vertices count] - 1;
        
        v = [[Vector3f alloc] initWithX:+4096 y:+4096 z:+4096];
        [vertices addObject:v];
        [v release];
        int ent = [vertices count] - 1;

        // create edges
        Edge* e = [[Edge alloc] initWithStartIndex:wsb endIndex:wst];
        [edges addObject:e];
        [e release];
        int wsbwst = [edges count] - 1;

        e = [[Edge alloc] initWithStartIndex:wsb endIndex:wnb];
        [edges addObject:e];
        [e release];
        int wsbwnb = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:wsb endIndex:esb];
        [edges addObject:e];
        [e release];
        int wsbesb = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:ent endIndex:enb];
        [edges addObject:e];
        [e release];
        int entenb = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:ent endIndex:est];
        [edges addObject:e];
        [e release];
        int entest = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:ent endIndex:wnt];
        [edges addObject:e];
        [e release];
        int entwnt = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:wst endIndex:wnt];
        [edges addObject:e];
        [e release];
        int wstwnt = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:wst endIndex:est];
        [edges addObject:e];
        [e release];
        int wstest = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:wnb endIndex:wnt];
        [edges addObject:e];
        [e release];
        int wnbwnt = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:wnb endIndex:enb];
        [edges addObject:e];
        [e release];
        int wnbenb = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:esb endIndex:est];
        [edges addObject:e];
        [e release];
        int esbest = [edges count] - 1;
        
        e = [[Edge alloc] initWithStartIndex:esb endIndex:enb];
        [edges addObject:e];
        [e release];
        int esbenb = [edges count] - 1;
        
        // create sides
        int eastEdges[] = {esbenb, entenb, entest, esbest};
        BOOL eastFlipped[] = {NO, YES, NO, YES};
        Side* eastSide = [[Side alloc] initWithEdges:eastEdges flipped:eastFlipped count:4];
        [sides addObject:eastSide];
        [sideToFace addObject:null];
        [eastSide release];
        
        int westEdges[] = {wsbwst, wstwnt, wnbwnt, wsbwnb};
        BOOL westFlipped[] = {NO, NO, YES, YES};
        Side* westSide = [[Side alloc] initWithEdges:westEdges flipped:westFlipped count:4];
        [sides addObject:westSide];
        [sideToFace addObject:null];
        [westSide release];
        
        int northEdges[] = {entwnt, wnbwnt, wnbenb, entenb};
        BOOL northFlipped[] = {NO, YES, NO, YES};
        Side* northSide = [[Side alloc] initWithEdges:northEdges flipped:northFlipped count:4];
        [sides addObject:northSide];
        [sideToFace addObject:null];
        [northSide release];
        
        int southEdges[] = {wsbesb, esbest, wstest, wsbwst};
        BOOL southFlipped[] = {NO, NO, YES, YES};
        Side* southSide = [[Side alloc] initWithEdges:southEdges flipped:southFlipped count:4];
        [sides addObject:southSide];
        [sideToFace addObject:null];
        [southSide release];
        
        int topEdges[] = {wstest, entest, entwnt, wstwnt};
        BOOL topFlipped[] = {NO, YES, NO, YES};
        Side* topSide = [[Side alloc] initWithEdges:topEdges flipped:topFlipped count:4];
        [sides addObject:topSide];
        [sideToFace addObject:null];
        [topSide release];
        
        int bottomEdges[] = {wsbwnb, wnbenb, esbenb, wsbesb};
        BOOL bottomFlipped[] = {NO, NO, YES, YES};
        Side* bottomSide = [[Side alloc] initWithEdges:bottomEdges flipped:bottomFlipped count:4];
        [sides addObject:bottomSide];
        [sideToFace addObject:null];
        [bottomSide release];
    }
    
    return self;
}

- (BOOL)cutWithFace:(Face *)face droppedFaces:(NSMutableArray **)droppedFaces {
    HalfSpace3D* halfSpace = [face halfSpace];
    
    int oldVertexCount = [vertices count];
    int oldEdgeCount = [edges count];
    
    // mark vertex statuses
    BOOL vertexStatus[oldVertexCount];
    EObjectStatus status;
    
    int p = -1;
    for (int i = 0; i < [vertices count]; i++) {
        if ([vertices objectAtIndex:i] != null) {
            Vector3f* vertex = [vertices objectAtIndex:i];
            vertexStatus[i] = [halfSpace containsPoint:vertex];
            if (p == -1)
                status = vertexStatus[i] ? OS_KEEP : OS_DROP;
            else if (vertexStatus[p] != vertexStatus[i])
                status = OS_SPLIT;
            p = i;
        }
    }

    if (status == OS_KEEP)
        return YES;
    
    if (status == OS_DROP)
        return NO;
    
    // mark edge statuses
    Line3D* line = [[Line3D alloc] init];
    EObjectStatus edgeStatus[oldEdgeCount];
    for (int i = 0; i < [edges count]; i++) {
        if ([edges objectAtIndex:i] != null) {
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
                
                int newVertexIndex;
                if ([freeVertexSlots count] > 0) {
                    newVertexIndex = [[freeVertexSlots lastObject] intValue];
                    [freeVertexSlots removeLastObject];
                    [vertices replaceObjectAtIndex:newVertexIndex withObject:newVertex];
                } else {
                    newVertexIndex = [vertices count];
                    [vertices addObject:newVertex];
                }
                
                if (!startStatus)
                    [edge setStartIndex:newVertexIndex];
                else
                    [edge setEndIndex:newVertexIndex];
            }
        }
    }
    [line release];
    
    // handle sides
    *droppedFaces = [[NSMutableArray alloc] init];
    for (int i = 0; i < [sides count]; i++) {
        Side* side = [sides objectAtIndex:i];
        if (side != null) {
            int newEdgeIndex;
            if ([freeEdgeSlots count] > 0)
                newEdgeIndex = [[freeEdgeSlots lastObject] intValue];
            else
                newEdgeIndex = [edges count];
            
            ESideUpdateResult result = [side updateEdges:edgeStatus newEdgeIndex:newEdgeIndex edges:edges];
            if (result.status == OS_DROP) {
                if ([sideToFace objectAtIndex:i] != null) {
                    Face* face = [sideToFace objectAtIndex:i];
                    [sideToFace replaceObjectAtIndex:i withObject:null];
                    [faceToSide removeObjectForKey:[face getId]];
                    [*droppedFaces addObject:face];
                }
                [sides replaceObjectAtIndex:i withObject:null];
                [freeSideSlots addObject:[NSNumber numberWithInt:i]];
            } else if (result.status == OS_SPLIT) {
                if ([freeEdgeSlots count] > 0) {
                    [freeEdgeSlots removeLastObject];
                    [edges replaceObjectAtIndex:newEdgeIndex withObject:result.newEdge];
                } else {
                    [edges addObject:result.newEdge];
                }
                [result.newEdge release];
            }
        }
    }

    // create a new side from the newly created edges
    int newSideEdgeCount = [edges count] - oldEdgeCount;
    int newSideEdges[newSideEdgeCount];
    BOOL newSideFlipped[newSideEdgeCount];
    
    newSideFlipped[0] = NO;
    for (int i = 0; i < newSideEdgeCount; i++)
        newSideEdges[i] = i + oldEdgeCount;

    // sort the edges to form a polygon
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

    Side* side = [[Side alloc] initWithEdges:newSideEdges flipped:newSideFlipped count:newSideEdgeCount];
    if ([freeSideSlots count] > 0) {
        int newSideIndex = [[freeSideSlots lastObject] intValue];
        [freeSideSlots removeLastObject];
        [sides replaceObjectAtIndex:newSideIndex withObject:side];
        [sideToFace replaceObjectAtIndex:newSideIndex withObject:face];
    } else {
        [sides addObject:side];
        [sideToFace addObject:face];
    }
    [faceToSide setObject:side forKey:[face getId]];

    for (int i = 0; i < oldEdgeCount; i++) {
        if (edgeStatus[i] == OS_DROP) {
            [edges replaceObjectAtIndex:i withObject:null];
            [freeEdgeSlots addObject:[NSNumber numberWithInt:i]];
        }
    }
    
    for (int i = 0; i < oldVertexCount; i++) {
        if (!vertexStatus[i]) {
            [vertices replaceObjectAtIndex:i withObject:null];
            [freeVertexSlots addObject:[NSNumber numberWithInt:i]];
        }
    }
    
    return YES;
}

- (NSArray *)verticesForFace:(Face *)face {
    Side* side = [faceToSide objectForKey:[face getId]];
    if (side == nil)
        [NSException raise:NSInvalidArgumentException format:@"Unknown face: %@", face];
    return [side verticesWidthEdgeArray:edges vertexArray:vertices];
}

- (void)dealloc {
    [sideToFace release];
    [faceToSide release];
    [sides release];
    [edges release];
    [vertices release];
    [freeVertexSlots release];
    [freeEdgeSlots release];
    [freeSideSlots release];
    [null release];
    [super dealloc];
}

@end

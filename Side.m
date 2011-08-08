//
//  Side.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Side.h"
#import "Edge.h"
#import "Vertex.h"
#import "MutableFace.h"
#import "PickingHit.h"

@interface Side (private)

- (EPlane)projectionPlane;

@end

@implementation Side (private)

- (EPlane)projectionPlane {
    TVector3f* norm = [face norm];
    switch (strongestComponentV3f(norm)) {
        case A_X:
            return P_YZ;
        case A_Y:
            return P_XZ;
        default:
            return P_XY;
    }
}

@end

@implementation Side

- (id)init {
    if ((self = [super init])) {
        mark = SM_NEW;
        vertices = [[NSMutableArray alloc] init];
        edges = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithEdges:(NSArray *)theEdges flipped:(BOOL*)flipped {
    NSAssert(theEdges != nil, @"edge array must not be nil");
    NSAssert([theEdges count] >= 3, @"edge array must contain at least three edges");
    NSAssert(flipped != NULL, @"flip flags array must not be NULL");
    
    if ((self = [self init])) {
        face = nil;
        for (int i = 0; i < [theEdges count]; i++) {
            Edge* edge = [theEdges objectAtIndex:i];
            if (!flipped[i])
                [edge setRightSide:self];
            else
                [edge setLeftSide:self];
            [edges addObject:edge];
            [vertices addObject:[edge startVertexForSide:self]];
        }
    }
    
    return self;
}

- (id)initWithFace:(MutableFace *)theFace edges:(NSArray *)theEdges {
    NSAssert(theFace != nil, @"face must not be nil");
    NSAssert(theEdges != nil, @"edge array must not be nil");
    NSAssert([theEdges count] >= 3, @"edge array must contain at least three edges");

    if ((self = [self init])) {
        face = [theFace retain];
        NSEnumerator* edgeEn = [theEdges objectEnumerator];
        Edge* edge;
        while ((edge = [edgeEn nextObject])) {
            [edge setLeftSide:self];
            [edges addObject:edge];
            [vertices addObject:[edge startVertexForSide:self]];
        }
        
        [face setVertices:vertices];
        [face setEdges:edges];
    }
    
    return self;
}

- (Edge *)split {
    int keep = 0;
    int drop = 0;
    int split = 0;
    int undecided = 0;
    Edge* undecidedEdge = nil;
    
    int splitIndex1 = -2;
    int splitIndex2 = -2;

    Edge* edge = [edges lastObject];
    EEdgeMark lastMark = [edge mark];
    for (int i = 0; i < [edges count]; i++) {
        edge = [edges objectAtIndex:i];
        EEdgeMark currentMark = [edge mark];
        if (currentMark == EM_SPLIT) {
            if ([[edge startVertexForSide:self] mark] == VM_KEEP)
                splitIndex1 = i;
            else
                splitIndex2 = i;
            split++;
        } else if (currentMark == EM_UNDECIDED) {
            undecided++;
            undecidedEdge = edge;
        } else if (currentMark == EM_KEEP) {
            if (lastMark == EM_DROP)
                splitIndex2 = i;
            keep++;
        } else if (currentMark == EM_DROP) {
            if (lastMark == EM_KEEP)
                splitIndex1 = i > 0 ? i - 1 : [edges count] - 1;
            drop++;
        }
        lastMark = currentMark;
    }
    
    if (keep == [edges count]) {
        mark = SM_KEEP;
        return nil;
    }
    
    if (undecided == 1 && keep == [edges count] - 1) {
        mark = SM_KEEP;
        return undecidedEdge;
    }
    
    if (drop + undecided == [edges count]) {
        mark = SM_DROP;
        return nil;
    }

    mark = SM_SPLIT;
    
    Vertex* startVertex = [[edges objectAtIndex:splitIndex1] endVertexForSide:self];
    Vertex* endVertex = [[edges objectAtIndex:splitIndex2] startVertexForSide:self];
    Edge* newEdge = [[Edge alloc] initWithStartVertex:startVertex endVertex:endVertex];
    [newEdge setRightSide:self];
    
    if (splitIndex2 > splitIndex1) {
        int num = splitIndex2 - splitIndex1 - 1;
        if (num > 0)
            [edges removeObjectsInRange:NSMakeRange(splitIndex1 + 1, num)];
        [edges insertObject:newEdge atIndex:splitIndex1 + 1];
    } else {
        int num = [edges count] - splitIndex1 - 1;
        if (num > 0)
            [edges removeObjectsInRange:NSMakeRange(splitIndex1 + 1, num)];
        num = splitIndex2;
        if (num > 0)
            [edges removeObjectsInRange:NSMakeRange(0, num)];
        [edges addObject:newEdge];
    }

    [vertices removeAllObjects];
    NSEnumerator* edgeEn = [edges objectEnumerator];
    while ((edge = [edgeEn nextObject]))
        [vertices addObject:[edge startVertexForSide:self]];
    
    [face setVertices:vertices];
    [face setEdges:edges];
    
    return [newEdge autorelease];
}

- (ESideMark)mark {
    return mark;
}

- (void)setMark:(ESideMark)theMark {
    mark = theMark;
}

- (NSArray *)vertices {
    return vertices;
}

- (NSArray *)edges {
    return edges;
}

- (MutableFace *)face {
    return face;
}

- (PickingHit *)pickWithRay:(TRay *)theRay {
    TVector3f* norm = [face norm];
    float d = dotV3f(norm, &theRay->direction);
    if (!fneg(d))
        return nil;
    
    TPlane* plane = [face boundary];
    float dist = intersectPlaneWithRay(plane, theRay);
    if (isnan(dist))
        return nil;
    
    EPlane cPlane = [self projectionPlane];
    TVector3f is, pis, v0, v1;

    rayPointAtDistance(theRay, dist, &is);
    projectOntoPlane(cPlane, &is, &pis);
    
    int c = 0;
    Vertex* v = [[self vertices] lastObject];
    projectOntoPlane(cPlane, [v vector], &v0);
    subV3f(&v0, &pis, &v0);
    
    NSEnumerator* vertexEn = [vertices objectEnumerator];
    while ((v = [vertexEn nextObject])) {
        projectOntoPlane(cPlane, [v vector], &v1);
        subV3f(&v1, &pis, &v1);
        
        if ((fzero(v0.x) && fzero(v0.y)) || (fzero(v1.x) && fzero(v1.y))) {
            // the point is identical to a polygon vertex, cancel search
            c = 1;
            break;
        }
        
        /*
         * A polygon edge intersects with the positive X axis if the
         * following conditions are met: The Y coordinates of its
         * vertices must have different signs (we assign a negative sign
         * to 0 here in order to count it as a negative number) and one
         * of the following two conditions must be met: Either the X
         * coordinates of the vertices are both positive or the X
         * coordinates of the edge have different signs (again, we
         * assign a negative sign to 0 here). In the latter case, we
         * must calculate the point of intersection between the edge and
         * the X axis and determine whether its X coordinate is positive
         * or zero.
         */
        
        // do the Y coordinates have different signs?
        if ((v0.y > 0 && v1.y <= 0) || (v0.y <= 0 && v1.y > 0)) {
            // Is segment entirely on the positive side of the X axis?
            if (v0.x > 0 && v1.x > 0) {
                c += 1; // edge intersects with the X axis
                // if not, do the X coordinates have different signs?
            } else if ((v0.x > 0 && v1.x <= 0) || (v0.x <= 0 && v1.x > 0)) {
                // calculate the point of intersection between the edge
                // and the X axis
                float x = -v0.y * (v1.x - v0.x) / (v1.y - v0.y) + v0.x;
                if (x >= 0)
                    c += 1; // edge intersects with the X axis
            }
        }
        
        v0 = v1;
    }
    
    if (c % 2 == 0)
        return nil;
    
    return [[[PickingHit alloc] initWithObject:face type:HT_FACE hitPoint:&is distance:dist] autorelease];
}

- (NSString *)description {
    NSMutableString* desc = [NSMutableString stringWithFormat:@"side mark: "];
    switch (mark) {
        case SM_KEEP:
            [desc appendFormat:@"SM_KEEP\n"];
            break;
        case SM_DROP:
            [desc appendFormat:@"SM_DROP\n"];
            break;
        case SM_SPLIT:
            [desc appendFormat:@"SM_SPLIT\n"];
            break;
        case SM_NEW:
            [desc appendFormat:@"SM_NEW\n"];
            break;
        case SM_UNKNOWN:
            [desc appendFormat:@"SM_UNKNOWN\n"];
            break;
        default:
            [desc appendFormat:@"invalid\n"];
            break;
    }
    
    NSEnumerator* eEn = [edges objectEnumerator];
    Edge* edge;
    while ((edge = [eEn nextObject]))
        [desc appendFormat:@"%@\n", edge];
    
    return desc;
}

- (void)dealloc {
    [face release];
    [vertices release];
    [edges release];
    [super dealloc];
}

@end

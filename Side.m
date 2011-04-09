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
#import "Vector3f.h"
#import "Vector2f.h"
#import "MutableFace.h"
#import "Ray3D.h"
#import "PickingHit.h"
#import "CoordinatePlane.h"
#import "Math.h"
#import "Plane3D.h"
#import "SegmentIterator.h"

@implementation Side

- (id)init {
    if (self = [super init]) {
        mark = SM_NEW;
        vertices = [[NSMutableArray alloc] init];
        edges = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithFace:(MutableFace *)theFace edges:(NSArray *)theEdges flipped:(BOOL*)flipped {
    if (self = [self init]) {
        face = [theFace retain];
        for (int i = 0; i < [theEdges count]; i++) {
            Edge* edge = [theEdges objectAtIndex:i];
            if (!flipped[i])
                [edge setRightSide:self];
            else
                [edge setLeftSide:self];
            [edges addObject:edge];
            [vertices addObject:[edge startVertexForSide:self]];
        }

        [face setVertices:vertices];
        [face setEdges:edges];
    }
    
    return self;
}

- (id)initWithFace:(MutableFace *)theFace edges:(NSArray *)theEdges {
    if (self = [self init]) {
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

    EEdgeMark lastMark = [[edges lastObject] mark];
    for (int i = 0; i < [edges count]; i++) {
        Edge* edge = [edges objectAtIndex:i];
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
    Edge* edge;
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

- (PickingHit *)pickWithRay:(Ray3D *)theRay {
    Vector3f* norm = [face norm];
    float d = [norm dot:[theRay direction]];
    if (!fneg(d))
        return nil;
    
    Plane3D* plane = [face boundary];
    float dist = [plane intersectWithRay:theRay];
    if (isnan(dist))
        return nil;
    
    Vector3f* is = [theRay pointAtDistance:dist];
    CoordinatePlane* cPlane = [CoordinatePlane projectionPlaneForNormal:norm];
    float isx = [cPlane xOf:is];
    float isy = [cPlane yOf:is];
    
    int c = 0;
    Vertex* v = [[self vertices] lastObject];
    float x0 = [cPlane xOf:[v vector]] - isx;
    float y0 = [cPlane yOf:[v vector]] - isy;
    
    NSEnumerator* vertexEn = [vertices objectEnumerator];
    while ((v = [vertexEn nextObject])) {
        float x1 = [cPlane xOf:[v vector]] - isx;
        float y1 = [cPlane yOf:[v vector]] - isy;
        
        if ((fzero(x0) && fzero(y0)) || (fzero(x1) && fzero(y1))) {
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
        if ((y0 > 0 && y1 <= 0) || (y0 <= 0 && y1 > 0)) {
            // Is segment entirely on the positive side of the X axis?
            if (x0 > 0 && x1 > 0) {
                c += 1; // edge intersects with the X axis
                // if not, do the X coordinates have different signs?
            } else if ((x0 > 0 && x1 <= 0) || (x0 <= 0 && x1 > 0)) {
                // calculate the point of intersection between the edge
                // and the X axis
                float x = -y0 * (x1 - x0) / (y1 - y0) + x0;
                if (x >= 0)
                    c += 1; // edge intersects with the X axis
            }
        }
        
        x0 = x1;
        y0 = y1;
    }
    
    if (c % 2 == 0)
        return nil;
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:is];
    [diff sub:[theRay origin]];
    
    float distance = [diff length];
    [diff release];
    
    return [[[PickingHit alloc] initWithObject:face type:HT_FACE hitPoint:is distance:distance] autorelease];
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

- (Vector3f *)center {
    if (center == nil) {
        NSEnumerator* vertexEn = [[self vertices] objectEnumerator];
        Vertex* vertex = [vertexEn nextObject];
        center = [[Vector3f alloc] initWithFloatVector:[vertex vector]];
        while ((vertex = [vertexEn nextObject]))
            [center add:[vertex vector]];
        
        [center scale:1.0f / [[self vertices] count]];
    }
    
    return center;
}

- (void)dealloc {
    [face release];
    [vertices release];
    [edges release];
    [center release];
    [super dealloc];
}

@end

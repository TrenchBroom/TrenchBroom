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
#import "SideEdge.h"
#import "Vector3f.h"
#import "Vector2f.h"
#import "MutableFace.h"
#import "Ray3D.h"
#import "PickingHit.h"
#import "CoordinatePlane.h"
#import "Math.h"
#import "Math3D.h"
#import "HalfSpace3D.h"
#import "Plane3D.h"
#import "SegmentIterator.h"
#import "MathCache.h"
#import "GridFigure.h"

@implementation Side

- (id)init {
    if (self = [super init]) {
        sideEdges = [[NSMutableArray alloc] init];
        mark = SM_NEW;
        grids = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithFace:(MutableFace *)theFace edges:(NSArray *)theEdges flipped:(BOOL*)flipped {
    if (theEdges == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge array must not be nil"];
    
    if (self = [self init]) {
        face = [theFace retain];
        for (int i = 0; i < [theEdges count]; i++) {
            Edge* edge = [theEdges objectAtIndex:i];
            SideEdge* sideEdge = [[SideEdge alloc] initWithEdge:edge flipped:flipped[i]];
            [sideEdges addObject:sideEdge];
            [sideEdge setSide:self];
            [sideEdge release];
        }
    }
    
    return self;
}

- (id)initWithFace:(MutableFace *)theFace sideEdges:(NSArray *)theEdges {
    if (theEdges == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge array must not be nil"];
    
    if (self = [self init]) {
        face = [theFace retain];

        NSEnumerator* edgeEn = [theEdges objectEnumerator];
        SideEdge* edge;
        while ((edge = [edgeEn nextObject])) {
            [sideEdges addObject:edge];
            [edge setSide:self];
        }
    }
    
    return self;
}

- (SideEdge *)split {
    EEdgeMark currentMark = [[sideEdges lastObject] mark];
    if (currentMark == EM_KEEP)
        mark = SM_KEEP;
    else if (currentMark == EM_DROP)
        mark = SM_DROP;
    else if (currentMark == EM_SPLIT)
        mark = SM_SPLIT;
    
    int splitIndex1, splitIndex2 = -1;

    for (int i = 0; i < [sideEdges count]; i++) {
        SideEdge* sideEdge = [sideEdges objectAtIndex:i];
        currentMark = [sideEdge mark];
        if (currentMark == EM_SPLIT) {
            if ([[sideEdge startVertex] mark] == VM_KEEP)
                splitIndex1 = i;
            else
                splitIndex2 = i;
        }
        
        if ((mark == SM_KEEP && currentMark != EM_KEEP) || 
            (mark == SM_DROP && currentMark != EM_DROP))
            mark = SM_SPLIT;
    }
    
    if (mark == SM_KEEP || mark == SM_DROP)
        return nil;

    [vertices release];
    vertices = nil;
    [edges release];
    edges = nil;
    [grids removeAllObjects];
    
    Vertex* startVertex = [[sideEdges objectAtIndex:splitIndex1] endVertex];
    Vertex* endVertex = [[sideEdges objectAtIndex:splitIndex2] startVertex];
    Edge* newEdge = [[Edge alloc] initWithStartVertex:startVertex endVertex:endVertex];
    SideEdge* sideEdge = [[SideEdge alloc] initWithEdge:newEdge flipped:NO];
    [sideEdge setSide:self];
    if (splitIndex2 > splitIndex1) {
        int num = splitIndex2 - splitIndex1 - 1;
        if (num > 0)
            [sideEdges removeObjectsInRange:NSMakeRange(splitIndex1 + 1, num)];
        [sideEdges insertObject:sideEdge atIndex:splitIndex1 + 1];
    } else {
        int num = [sideEdges count] - splitIndex1 - 1;
        if (num > 0)
            [sideEdges removeObjectsInRange:NSMakeRange(splitIndex1 + 1, num)];
        num = splitIndex2;
        if (num > 0)
            [sideEdges removeObjectsInRange:NSMakeRange(0, num)];
        [sideEdges addObject:sideEdge];
    }
    [sideEdge release];
    
    SideEdge* opposingEdge = [[SideEdge alloc] initWithEdge:newEdge flipped:YES];
    [newEdge release];
    
    return [opposingEdge autorelease];
}

- (ESideMark)mark {
    return mark;
}

- (void)setMark:(ESideMark)theMark {
    mark = theMark;
}

- (NSArray *)vertices {
    if (vertices == nil) {
        vertices = [[NSMutableArray alloc] initWithCapacity:[sideEdges count]];
    
        NSEnumerator* sideEdgeEn = [sideEdges objectEnumerator];
        SideEdge* sideEdge;
        while ((sideEdge = [sideEdgeEn nextObject]))
            [vertices addObject:[[sideEdge startVertex] vector]];
    }
    
    return vertices;
}

- (NSArray *)edges {
    if (edges == nil) {
        edges = [[NSMutableArray alloc] initWithCapacity:[sideEdges count]];
        
        NSEnumerator* sideEdgeEn = [sideEdges objectEnumerator];
        SideEdge* sideEdge;
        while ((sideEdge = [sideEdgeEn nextObject]))
            [edges addObject:[sideEdge edge]];
    }
    
    return edges;
}

- (NSArray *)gridWithSize:(int)gridSize {
    NSNumber* key = [NSNumber numberWithInt:gridSize];
    NSMutableArray* grid = [grids objectForKey:key];
    if (grid == nil) {
        Vector3f* norm = [face norm];
        CoordinatePlane* plane = [CoordinatePlane projectionPlaneForNormal:norm];
        
        NSMutableArray* pVertices = [[NSMutableArray alloc] initWithCapacity:[[self vertices] count]];
        
        float sx = FLT_MAX;
        float sy = FLT_MAX;
        float lx = -FLT_MAX;
        float ly = -FLT_MAX;
        
        NSEnumerator* vertexEn = [[self vertices] objectEnumerator];
        Vector3f* vertex;
        while ((vertex = [vertexEn nextObject])) {
            Vector3f* pVertex = [plane project:vertex];
            [pVertices addObject:pVertex];
            
            float x = [pVertex x];
            float y = [pVertex y];
            
            if (x < sx)
                sx = x;
            if (x > lx)
                lx = x;
            if (y < sy)
                sy = y;
            if (y > ly)
                ly = y;
        }
        
        int gridNo = sx / gridSize;
        if (sx > 0)
            gridNo++;
        float gridX = gridNo * gridSize; // first grid line to intersect with polygon
        if (feq(gridX, sx))
            gridX += gridSize;
        
        BOOL clockwise = [plane clockwise:norm];
        grid = [[NSMutableArray alloc] init];
        
        SegmentIterator* si = [[SegmentIterator alloc] initWithVertices:pVertices vertical:NO clockwise:clockwise];
        while (flt(gridX, lx)) {
            
            Vector3f* ls = [si forwardLeftTo:gridX];
            Vector3f* rs = [si forwardRightTo:gridX];
            if (ls == nil || rs == nil)
                break;
            
            Vector3f* le = [si nextLeft];
            Vector3f* re = [si nextRight];
            
            float lx = gridX;
            float ly = ([le y] - [ls y]) * (gridX - [ls x]) / ([le x] - [ls x]) + [ls y];
            float lz = ([le z] - [ls z]) * (gridX - [ls x]) / ([le x] - [ls x]) + [ls z];
            
            Vector3f* lgv = [[Vector3f alloc] init];
            [plane set:lgv toX:lx y:ly z:lz];
            
            float rx = gridX;
            float ry = ([re y] - [rs y]) * (gridX - [rs x]) / ([re x] - [rs x]) + [rs y];
            float rz = ([re z] - [rs z]) * (gridX - [rs x]) / ([re x] - [rs x]) + [rs z];
            
            Vector3f* rgv = [[Vector3f alloc] init];
            [plane set:rgv toX:rx y:ry z:rz];
            
            GridFigure* figure = [[GridFigure alloc] initWithPoint1:lgv point2:rgv];
            [grid addObject:figure];

            [figure release];
            [lgv release];
            [rgv release];
            
            gridX += gridSize;
        }
        [si release];
        
        gridNo = sy / gridSize;
        if (sy > 0)
            gridNo++;
        float gridY = gridNo * gridSize;
        if (feq(gridY, sy))
            gridY += gridSize;
        
        si = [[SegmentIterator alloc] initWithVertices:pVertices vertical:YES clockwise:clockwise];
        
        while (flt(gridY, ly)) {
            Vector3f* ls = [si forwardLeftTo:gridY];
            Vector3f* rs = [si forwardRightTo:gridY];
            if (ls == nil || rs == nil)
                break;
            
            Vector3f* le = [si nextLeft];
            Vector3f* re = [si nextRight];
            
            float lx = ([le x] - [ls x]) * (gridY - [ls y]) / ([le y] - [ls y]) + [ls x];
            float ly = gridY;
            float lz = ([le z] - [ls z]) * (gridY - [ls y]) / ([le y] - [ls y]) + [ls z];
            
            Vector3f* lgv = [[Vector3f alloc] init];
            [plane set:lgv toX:lx y:ly z:lz];
            
            float rx = ([re x] - [rs x]) * (gridY - [rs y]) / ([re y] - [rs y]) + [rs x];
            float ry = gridY;
            float rz = ([re z] - [rs z]) * (gridY - [rs y]) / ([re y] - [rs y]) + [rs z];
            
            Vector3f* rgv = [[Vector3f alloc] init];
            [plane set:rgv toX:rx y:ry z:rz];
            
            GridFigure* figure = [[GridFigure alloc] initWithPoint1:lgv point2:rgv];
            [grid addObject:figure];
            
            [figure release];
            [lgv release];
            [rgv release];
            
            gridY += gridSize;
        }
        [si release];
        
        [pVertices release];
        
        [grids setObject:grid forKey:key];
    }
    
    return grid;
}

- (MutableFace *)face {
    return face;
}

- (PickingHit *)pickWithRay:(Ray3D *)theRay {
    Vector3f* norm = [face norm];
    float d = [norm dot:[theRay direction]];
    if (!fneg(d))
        return nil;
    
    Plane3D* plane = [[face halfSpace] boundary];
    float dist = [plane intersectWithRay:theRay];
    if (isnan(dist))
        return nil;
    
    Vector3f* is = [theRay pointAtDistance:dist];
    CoordinatePlane* cPlane = [CoordinatePlane projectionPlaneForNormal:norm];
    float isx = [cPlane xOf:is];
    float isy = [cPlane yOf:is];
    
    int c = 0;
    Vector3f* v = [vertices lastObject];
    float x0 = [cPlane xOf:v] - isx;
    float y0 = [cPlane yOf:v] - isy;
    
    NSEnumerator* vertexEn = [vertices objectEnumerator];
    while ((v = [vertexEn nextObject])) {
        float x1 = [cPlane xOf:v] - isx;
        float y1 = [cPlane yOf:v] - isy;
        
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
    
    MathCache* cache = [MathCache sharedCache];
    Vector3f* diff = [cache vector3f];
    [diff setFloat:is];
    [diff sub:[theRay origin]];
    
    float distance = [diff length];
    [cache returnVector3f:diff];
    
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
    
    NSEnumerator* eEn = [sideEdges objectEnumerator];
    SideEdge* sideEdge;
    while ((sideEdge = [eEn nextObject]))
        [desc appendFormat:@"%@\n", sideEdge];
    
    return desc;
}

- (Vector3f *)center {
    if (center == nil) {
        NSEnumerator* vertexEn = [[self vertices] objectEnumerator];
        Vector3f* vertex = [vertexEn nextObject];
        center = [[Vector3f alloc] initWithFloatVector:vertex];
        while ((vertex = [vertexEn nextObject]))
            [center add:vertex];
        
        [center scale:1.0f / [[self vertices] count]];
    }
    
    return center;
}

- (void)dealloc {
    [face release];
    [vertices release];
    [center release];
    [sideEdges release];
    [grids release];
    [super dealloc];
}

@end

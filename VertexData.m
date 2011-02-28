//
//  VertexData2.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "VertexData.h"
#import "Vertex.h"
#import "Vector2f.h"
#import "Vector3f.h"
#import "BoundingBox.h"
#import "CoordinatePlane.h"
#import "Edge.h"
#import "MutableFace.h"
#import "HalfSpace3D.h"
#import "Math.h"
#import "Math3D.h"
#import "PickingHit.h"
#import "Plane3D.h"
#import "Ray3D.h"
#import "Side.h"
#import "SideEdge.h"
#import "SegmentIterator.h"

@implementation VertexData

- (id)init {
    if (self = [super init]) {
        vertices = [[NSMutableArray alloc] init];
        edges = [[NSMutableArray alloc] init];
        sides = [[NSMutableArray alloc] init];
        faceToSide = [[NSMutableDictionary alloc] init];
        centers = [[NSMutableDictionary alloc] init];
        bounds = nil;
        
        // initialize as huge cube
        Vector3f* v = [[Vector3f alloc] initWithX:-4096 y:-4096 z:-4096];
        Vertex* wsb = [[Vertex alloc] initWithVector:v];
        [vertices addObject:wsb];
        [wsb release];
        [v release];
        
        v = [[Vector3f alloc] initWithX:-4096 y:-4096 z:+4096];
        Vertex* wst = [[Vertex alloc] initWithVector:v];
        [vertices addObject:wst];
        [wst release];
        [v release];
        
        v = [[Vector3f alloc] initWithX:-4096 y:+4096 z:-4096];
        Vertex* wnb = [[Vertex alloc] initWithVector:v];
        [vertices addObject:wnb];
        [wnb release];
        [v release];
        
        v = [[Vector3f alloc] initWithX:-4096 y:+4096 z:+4096];
        Vertex* wnt = [[Vertex alloc] initWithVector:v];
        [vertices addObject:wnt];
        [wnt release];
        [v release];
        
        v = [[Vector3f alloc] initWithX:+4096 y:-4096 z:-4096];
        Vertex* esb = [[Vertex alloc] initWithVector:v];
        [vertices addObject:esb];
        [esb release];
        [v release];
        
        v = [[Vector3f alloc] initWithX:+4096 y:-4096 z:+4096];
        Vertex* est = [[Vertex alloc] initWithVector:v];
        [vertices addObject:est];
        [est release];
        [v release];
        
        v = [[Vector3f alloc] initWithX:+4096 y:+4096 z:-4096];
        Vertex* enb = [[Vertex alloc] initWithVector:v];
        [vertices addObject:enb];
        [enb release];
        [v release];
        
        v = [[Vector3f alloc] initWithX:+4096 y:+4096 z:+4096];
        Vertex* ent = [[Vertex alloc] initWithVector:v];
        [vertices addObject:ent];
        [ent release];
        [v release];
        
        // create edges
        Edge* esbwsb = [[Edge alloc] initWithStartVertex:esb endVertex:wsb];
        [edges addObject:esbwsb];
        [esbwsb release];
        
        Edge* wsbwst = [[Edge alloc] initWithStartVertex:wsb endVertex:wst];
        [edges addObject:wsbwst];
        [wsbwst release];
        
        Edge* wstest = [[Edge alloc] initWithStartVertex:wst endVertex:est];
        [edges addObject:wstest];
        [wstest release];
        
        Edge* estesb = [[Edge alloc] initWithStartVertex:est endVertex:esb];
        [edges addObject:estesb];
        [estesb release];
        
        Edge* wnbenb = [[Edge alloc] initWithStartVertex:wnb endVertex:enb];
        [edges addObject:wnbenb];
        [wnbenb release];
        
        Edge* enbent = [[Edge alloc] initWithStartVertex:enb endVertex:ent];
        [edges addObject:enbent];
        [enbent release];
        
        Edge* entwnt = [[Edge alloc] initWithStartVertex:ent endVertex:wnt];
        [edges addObject:entwnt];
        [entwnt release];
        
        Edge* wntwnb = [[Edge alloc] initWithStartVertex:wnt endVertex:wnb];
        [edges addObject:wntwnb];
        [wntwnb release];

        Edge* enbesb = [[Edge alloc] initWithStartVertex:enb endVertex:esb];
        [edges addObject:enbesb];
        [enbesb release];
        
        Edge* estent = [[Edge alloc] initWithStartVertex:est endVertex:ent];
        [edges addObject:estent];
        [estent release];
        
        Edge* wsbwnb = [[Edge alloc] initWithStartVertex:wsb endVertex:wnb];
        [edges addObject:wsbwnb];
        [wsbwnb release];
        
        Edge* wntwst = [[Edge alloc] initWithStartVertex:wnt endVertex:wst];
        [edges addObject:wntwst];
        [wntwst release];
                
        // create sides
        NSArray* southEdges = [[NSArray alloc] initWithObjects:esbwsb, wsbwst, wstest, estesb, nil];
        BOOL southFlipped[] = {NO, NO, NO, NO};
        Side* southSide = [[Side alloc] initWithFace:nil edges:southEdges flipped:southFlipped];
        [sides addObject:southSide];
        [southSide release];
        [southEdges release];
        
        NSArray* northEdges = [[NSArray alloc] initWithObjects:wnbenb, enbent, entwnt, wntwnb, nil];
        BOOL northFlipped[] = {NO, NO, NO, NO};
        Side* northSide = [[Side alloc] initWithFace:nil edges:northEdges flipped:northFlipped];
        [sides addObject:northSide];
        [northSide release];
        [northEdges release];

        NSArray* westEdges = [[NSArray alloc] initWithObjects:wsbwnb, wntwnb, wntwst, wsbwst, nil];
        BOOL westFlipped[] = {NO, YES, NO, YES};
        Side* westSide = [[Side alloc] initWithFace:nil edges:westEdges flipped:westFlipped];
        [sides addObject:westSide];
        [westSide release];
        [westEdges release];
        
        NSArray* eastEdges = [[NSArray alloc] initWithObjects:enbesb, estesb, estent, enbent, nil];
        BOOL eastFlipped[] = {NO, YES, NO, YES};
        Side* eastSide = [[Side alloc] initWithFace:nil edges:eastEdges flipped:eastFlipped];
        [sides addObject:eastSide];
        [eastSide release];
        [eastEdges release];
        
        NSArray* topEdges = [[NSArray alloc] initWithObjects:wstest, wntwst, entwnt, estent, nil];
        BOOL topFlipped[] = {YES, YES, YES, YES};
        Side* topSide = [[Side alloc] initWithFace:nil edges:topEdges flipped:topFlipped];
        [sides addObject:topSide];
        [topSide release];
        [topEdges release];
        
        NSArray* bottomEdges = [[NSArray alloc] initWithObjects:esbwsb, enbesb, wnbenb, wsbwnb, nil];
        BOOL bottomFlipped[] = {YES, YES, YES, YES};
        Side* bottomSide = [[Side alloc] initWithFace:nil edges:bottomEdges flipped:bottomFlipped];
        [sides addObject:bottomSide];
        [bottomSide release];
        [bottomEdges release];
    }
    
    return self;
}

- (id)initWithFaces:(NSArray *)faces droppedFaces:(NSMutableArray **)droppedFaces {
    if (faces == nil)
        [NSException raise:NSInvalidArgumentException format:@"face array must not be nil"];

    if (self = [self init]) {
        NSEnumerator* faceEn = [faces objectEnumerator];
        MutableFace * face;
        while ((face = [faceEn nextObject])) {
            if (![self cutWithFace:face droppedFaces:droppedFaces]) {
                [self release];
                return nil;
            }
        }
    }
    
    return self;
}

- (BOOL)cutWithFace:(MutableFace *)face droppedFaces:(NSMutableArray **)droppedFaces {
    HalfSpace3D* halfSpace = [face halfSpace];
    
    // mark vertices and brush
    NSEnumerator* vEn = [vertices objectEnumerator];
    Vertex* vertex = [vEn nextObject];
    EVertexMark vertexMark = [halfSpace containsPoint:[vertex vector]] ? VM_KEEP : VM_DROP;
    [vertex setMark:vertexMark];
    
    EBrushMark brushMark;
    if (vertexMark == VM_KEEP)
        brushMark = BM_KEEP;
    else if (vertexMark == VM_DROP)
        brushMark = BM_DROP;
                              
    while ((vertex = [vEn nextObject])) {
        vertexMark = [halfSpace containsPoint:[vertex vector]] ? VM_KEEP : VM_DROP;
        [vertex setMark:vertexMark];
        if (brushMark == BM_KEEP && vertexMark != VM_KEEP)
            brushMark = BM_SPLIT;
        else if (brushMark == BM_DROP && vertexMark != VM_DROP)
            brushMark = BM_SPLIT;
    }

    if (brushMark == VM_KEEP)
        return YES;
    else if (brushMark == VM_DROP)
        return NO;
    
    // mark and split edges
    NSEnumerator* eEn = [edges objectEnumerator];
    Edge* edge;
    
    while ((edge = [eEn nextObject])) {
        [edge updateMark];
        if ([edge mark] == EM_SPLIT) {
            Vertex* newVertex = [edge splitAt:[halfSpace boundary]];
            [vertices addObject:newVertex];
        }
    }
    
    // mark, split and drop sides
    if (*droppedFaces == nil)
        *droppedFaces = [NSMutableArray array];
    
    NSMutableArray* newEdges = [[NSMutableArray alloc] init];
    for (int i = 0; i < [sides count]; i++) {
        Side* side = [sides objectAtIndex:i];
        SideEdge* newEdge = [side split];
        if ([side mark] == SM_DROP) {
            id <Face> face = [side face];
            if (face != nil)
                [*droppedFaces addObject:face];
            [sides removeObjectAtIndex:i--];
        } else if ([side mark] == SM_SPLIT) {
            [edges addObject:[newEdge edge]];
            [newEdges addObject:newEdge];
            [side setMark:SM_UNKNOWN];
        } else {
            [side setMark:SM_UNKNOWN];
        }
    }
    
    // create new side from newly created edge
    // first, sort the new edges to form a polygon
    for (int i = 0; i < [newEdges count] - 1; i++) {
        SideEdge* edge = [newEdges objectAtIndex:i];
        for (int j = i + 2; j < [newEdges count]; j++) {
            SideEdge* candidate = [newEdges objectAtIndex:j];
            if ([edge endVertex] == [candidate startVertex]) {
                [newEdges exchangeObjectAtIndex:j withObjectAtIndex:i + 1];
                break;
            }
        }
    }
    
    // now create the side
    Side* side = [[Side alloc] initWithFace:face sideEdges:newEdges];
    [sides addObject:side];
    [faceToSide setObject:side forKey:[face faceId]];
    [newEdges release];
    [side release];
    
    // clean up
    for (int i = 0; i < [vertices count]; i++) {
        vertex = [vertices objectAtIndex:i];
        if ([vertex mark] == VM_DROP)
            [vertices removeObjectAtIndex:i--];
        else
            [vertex setMark:VM_UNKNOWN];
    }
    
    for (int i = 0; i < [edges count]; i++) {
        edge = [edges objectAtIndex:i];
        if ([edge mark] == EM_DROP)
            [edges removeObjectAtIndex:i--];
        else
            [edge updateMark];

    }
    
    [center release];
    center = nil;
    
    [bounds release];
    bounds = nil;
    
    return YES;
}

- (BoundingBox *)bounds {
    if (bounds == nil) {
        NSEnumerator* vertexEn = [vertices objectEnumerator];
        Vertex* vertex = [vertexEn nextObject];
        
        Vector3f* min = [[Vector3f alloc] initWithFloatVector:[vertex vector]];
        Vector3f* max = [[Vector3f alloc] initWithFloatVector:[vertex vector]];
        
        while ((vertex = [vertexEn nextObject])) {
            Vector3f* vector = [vertex vector];
            if ([vector x] < [min x])
                [min setX:[vector x]];
            if ([vector y] < [min y])
                [min setY:[vector y]];
            if ([vector z] < [min z])
                [min setZ:[vector z]];
            
            if ([vector x] > [max x])
                [max setX:[vector x]];
            if ([vector y] > [max y])
                [max setY:[vector y]];
            if ([vector z] > [max z])
                [max setZ:[vector z]];
        }
        
        bounds = [[BoundingBox alloc] initWithMin:min max:max];

        [min release];
        [max release];
    }
    
    return bounds;
}

- (Vector3f *)center {
    if (center == nil) {
        NSEnumerator* vertexEn = [vertices objectEnumerator];
        Vertex* vertex = [vertexEn nextObject];
        center = [[Vector3f alloc] initWithFloatVector:[vertex vector]];
        
        while ((vertex = [vertexEn nextObject]))
            [center add:[vertex vector]];
        
        [center scale:1.0f / [vertices count]];
    }
    
    return center;
}

- (NSArray *)verticesForFace:(MutableFace *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    Side* side = [faceToSide objectForKey:[face faceId]];
    if (side == nil)
        [NSException raise:NSInvalidArgumentException format:@"no vertex data for face %@", face];
    
    return [side vertices];
}

- (NSArray *)gridForFace:(MutableFace *)face gridSize:(int)gridSize {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];

    Side* side = [faceToSide objectForKey:[face faceId]];
    if (side == nil)
        [NSException raise:NSInvalidArgumentException format:@"no vertex data for face %@", face];

    Vector3f* norm = [face norm];
    CoordinatePlane* plane = [CoordinatePlane projectionPlaneForNormal:norm];
    
    NSMutableArray* pVertices = [[NSMutableArray alloc] initWithCapacity:[[face vertices] count]];
    
    float sx = FLT_MAX;
    float sy = FLT_MAX;
    float lx = -FLT_MAX;
    float ly = -FLT_MAX;
    
    NSEnumerator* vertexEn = [[face vertices] objectEnumerator];
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
    NSMutableArray* gVertices = [[NSMutableArray alloc] init];

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
        
        [gVertices addObject:lgv];
        [gVertices addObject:rgv];
        
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
        
        [gVertices addObject:lgv];
        [gVertices addObject:rgv];
        
        [lgv release];
        [rgv release];
        
        gridY += gridSize;
    }
    [si release];

    [pVertices release];
    return gVertices;
}

- (NSArray *)verticesForWireframe {
    NSMutableArray* result = [[NSMutableArray alloc] initWithCapacity:2 * [self edgeCount]];
    
    NSEnumerator* edgeEn = [edges objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject])) {
        Vertex* startVertex = [edge startVertex];
        Vertex* endVertex = [edge endVertex];
        
        [result addObject:[startVertex vector]];
        [result addObject:[endVertex vector]];
    }
    
    return [result autorelease];
}

- (int)edgeCount {
    return [edges count];
}

- (Vector3f *)centerOfFace:(MutableFace *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    
    Side* side = [faceToSide objectForKey:[face faceId]];
    if (side == nil)
        [NSException raise:NSInvalidArgumentException format:@"face %@ does not belong to %@", face, self];
    
    return [side center];
}

- (PickingHit *)pickFace:(MutableFace *)theFace withRay:(Ray3D *)theRay {
    if (theFace == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    if (theRay == nil)
        [NSException raise:NSInvalidArgumentException format:@"ray must not be nil"];
    
    Side* side = [faceToSide objectForKey:[theFace faceId]];
    return [side pickWithRay:theRay];
}

- (void)dealloc {
    [centers release];
    [faceToSide release];
    [sides release];
    [edges release];
    [vertices release];
    [bounds release];
    [center release];
    [super dealloc];
}

@end

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
#import "Math.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Plane3D.h"
#import "Ray3D.h"
#import "Side.h"
#import "SegmentIterator.h"

@implementation VertexData

- (id)init {
    if (self = [super init]) {
        vertices = [[NSMutableArray alloc] init];
        edges = [[NSMutableArray alloc] init];
        sides = [[NSMutableArray alloc] init];
        bounds = nil;
        
        // initialize as huge cube
        Vector3f* v = [[Vector3f alloc] initWithFloatX:-4096 y:-4096 z:-4096];
        Vertex* esb = [[Vertex alloc] initWithVector:v];
        [vertices addObject:esb];
        [esb release];
        [v release];
        
        v = [[Vector3f alloc] initWithFloatX:-4096 y:-4096 z:+4096];
        Vertex* est = [[Vertex alloc] initWithVector:v];
        [vertices addObject:est];
        [est release];
        [v release];
        
        v = [[Vector3f alloc] initWithFloatX:-4096 y:+4096 z:-4096];
        Vertex* enb = [[Vertex alloc] initWithVector:v];
        [vertices addObject:enb];
        [enb release];
        [v release];
        
        v = [[Vector3f alloc] initWithFloatX:-4096 y:+4096 z:+4096];
        Vertex* ent = [[Vertex alloc] initWithVector:v];
        [vertices addObject:ent];
        [ent release];
        [v release];
        
        v = [[Vector3f alloc] initWithFloatX:+4096 y:-4096 z:-4096];
        Vertex* wsb = [[Vertex alloc] initWithVector:v];
        [vertices addObject:wsb];
        [wsb release];
        [v release];
        
        v = [[Vector3f alloc] initWithFloatX:+4096 y:-4096 z:+4096];
        Vertex* wst = [[Vertex alloc] initWithVector:v];
        [vertices addObject:wst];
        [wst release];
        [v release];
        
        v = [[Vector3f alloc] initWithFloatX:+4096 y:+4096 z:-4096];
        Vertex* wnb = [[Vertex alloc] initWithVector:v];
        [vertices addObject:wnb];
        [wnb release];
        [v release];
        
        v = [[Vector3f alloc] initWithFloatX:+4096 y:+4096 z:+4096];
        Vertex* wnt = [[Vertex alloc] initWithVector:v];
        [vertices addObject:wnt];
        [wnt release];
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
        NSArray* southEdges = [[NSArray alloc] initWithObjects:esbwsb, estesb, wstest, wsbwst, nil];
        BOOL southFlipped[] = {YES, YES, YES, YES};
        Side* southSide = [[Side alloc] initWithFace:nil edges:southEdges flipped:southFlipped];
        [sides addObject:southSide];
        [southSide release];
        [southEdges release];
        
        NSArray* northEdges = [[NSArray alloc] initWithObjects:wnbenb, wntwnb, entwnt, enbent, nil];
        BOOL northFlipped[] = {YES, YES, YES, YES};
        Side* northSide = [[Side alloc] initWithFace:nil edges:northEdges flipped:northFlipped];
        [sides addObject:northSide];
        [northSide release];
        [northEdges release];

        NSArray* westEdges = [[NSArray alloc] initWithObjects:wsbwnb, wsbwst, wntwst, wntwnb, nil];
        BOOL westFlipped[] = {YES, NO, YES, NO};
        Side* westSide = [[Side alloc] initWithFace:nil edges:westEdges flipped:westFlipped];
        [sides addObject:westSide];
        [westSide release];
        [westEdges release];
        
        NSArray* eastEdges = [[NSArray alloc] initWithObjects:enbesb, enbent, estent, estesb, nil];
        BOOL eastFlipped[] = {YES, NO, YES, NO};
        Side* eastSide = [[Side alloc] initWithFace:nil edges:eastEdges flipped:eastFlipped];
        [sides addObject:eastSide];
        [eastSide release];
        [eastEdges release];
        
        NSArray* topEdges = [[NSArray alloc] initWithObjects:wstest, estent, entwnt, wntwst, nil];
        BOOL topFlipped[] = {NO, NO, NO, NO};
        Side* topSide = [[Side alloc] initWithFace:nil edges:topEdges flipped:topFlipped];
        [sides addObject:topSide];
        [topSide release];
        [topEdges release];
        
        NSArray* bottomEdges = [[NSArray alloc] initWithObjects:esbwsb, wsbwnb, wnbenb, enbesb, nil];
        BOOL bottomFlipped[] = {NO, NO, NO, NO};
        Side* bottomSide = [[Side alloc] initWithFace:nil edges:bottomEdges flipped:bottomFlipped];
        [sides addObject:bottomSide];
        [bottomSide release];
        [bottomEdges release];
    }
    
    return self;
}

- (id)initWithFaces:(NSArray *)faces droppedFaces:(NSMutableSet **)droppedFaces {
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

- (BOOL)cutWithFace:(MutableFace *)face droppedFaces:(NSMutableSet **)droppedFaces {
    Plane3D* plane = [face boundary];
    
    int keep = 0;
    int drop = 0;
    int undecided = 0;
    
    // mark vertices
    NSEnumerator* vEn = [vertices objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vEn nextObject])) {
        EPointStatus vertexStatus = [plane pointStatus:[vertex vector]];
        EVertexMark vertexMark;
        if (vertexStatus == PS_ABOVE) {
            vertexMark = VM_DROP;
            drop++;
        } else if (vertexStatus == PS_BELOW) {
            vertexMark = VM_KEEP;
            keep++;
        } else {
            vertexMark = VM_UNDECIDED;
            undecided++;
        }
        [vertex setMark:vertexMark];
    }

    if (keep + undecided == [vertices count]) {
        if (*droppedFaces == nil)
            *droppedFaces = [NSMutableSet set];
        [*droppedFaces addObject:face];
        return YES;
    }
    
    if (drop + undecided == [vertices count])
        return NO;
    
    // mark and split edges
    NSEnumerator* eEn = [edges objectEnumerator];
    Edge* edge;
    while ((edge = [eEn nextObject])) {
        [edge updateMark];
        if ([edge mark] == EM_SPLIT) {
            Vertex* newVertex = [edge splitAt:plane];
            [vertices addObject:newVertex];
        }
    }
    
    // mark, split and drop sides
    NSMutableArray* newEdges = [[NSMutableArray alloc] init];
    for (int i = 0; i < [sides count]; i++) {
        Side* side = [sides objectAtIndex:i];
        Edge* newEdge = [side split];
        if ([side mark] == SM_DROP) {
            id <Face> face = [side face];
            if (face != nil) {
                if (*droppedFaces == nil)
                    *droppedFaces = [NSMutableSet set];
                [*droppedFaces addObject:face];
            }
            [sides removeObjectAtIndex:i--];
        } else if ([side mark] == SM_SPLIT) {
            [edges addObject:newEdge];
            [newEdges addObject:newEdge];
            [side setMark:SM_UNKNOWN];
        } else if ([side mark] == SM_KEEP && newEdge != nil) {
            // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
            if ([newEdge rightSide] != side)
                [newEdge flip];
            [newEdges addObject:newEdge];
            [side setMark:SM_UNKNOWN];
        } else {
            [side setMark:SM_UNKNOWN];
        }
    }
    
    // create new side from newly created edge
    // first, sort the new edges to form a polygon in clockwise order
    for (int i = 0; i < [newEdges count] - 1; i++) {
        Edge* edge = [newEdges objectAtIndex:i];
        for (int j = i + 2; j < [newEdges count]; j++) {
            Edge* candidate = [newEdges objectAtIndex:j];
            if ([edge startVertex] == [candidate endVertex]) {
                [newEdges exchangeObjectAtIndex:j withObjectAtIndex:i + 1];
                break;
            }
        }
    }
    
    // now create the side
    Side* side = [[Side alloc] initWithFace:face edges:newEdges];
    [sides addObject:side];
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
            [edge clearMark];

    }
    
    [center release];
    center = nil;
    
    [bounds release];
    bounds = nil;
    
    [pickingBounds release];
    pickingBounds = nil;
    
    return YES;
}

- (NSArray *)vertices {
    return vertices;
}

- (NSArray *)edges {
    return edges;
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

- (int)edgeCount {
    return [edges count];
}

- (void)pick:(Ray3D *)theRay hitList:(PickingHitList *)theHitList {
    NSEnumerator* sideEn = [sides objectEnumerator];
    Side* side;
    PickingHit* faceHit = nil;
    while ((side = [sideEn nextObject]) && faceHit == nil)
        faceHit = [side pickWithRay:theRay];
    
    if (faceHit != nil) {
        PickingHit* brushHit = [[PickingHit alloc] initWithObject:[[faceHit object] brush] type:HT_BRUSH hitPoint:[faceHit hitPoint] distance:[faceHit distance]];
        [theHitList addHit:brushHit];
        [theHitList addHit:faceHit];
        [brushHit release];
    }
}

- (BoundingBox *)pickingBounds {
    if (pickingBounds == nil) {
        pickingBounds = [[BoundingBox alloc] initWithBounds:[self bounds]];
        NSEnumerator* edgeEn = [edges objectEnumerator];
        Edge* edge;
        while ((edge = [edgeEn nextObject]))
            [edge expandBounds:pickingBounds];
    }
    
    return pickingBounds;
}

- (void)dealloc {
    [sides release];
    [edges release];
    [vertices release];
    [bounds release];
    [pickingBounds release];
    [center release];
    [super dealloc];
}

@end

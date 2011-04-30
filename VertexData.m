//
//  VertexData2.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "VertexData.h"
#import "Vertex.h"
#import "Edge.h"
#import "MutableFace.h"
#import "Math.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Side.h"

@interface VertexData (private)

- (void)validate;

@end

@implementation VertexData (private)

- (void)validate {
    if (!valid) {
        NSEnumerator* vertexEn = [vertices objectEnumerator];
        Vertex* vertex = [vertexEn nextObject];
        
        bounds.min = *[vertex vector];
        bounds.max = *[vertex vector];        
        center = *[vertex vector];
        
        while ((vertex = [vertexEn nextObject])) {
            mergeBoundsWithPoint(&bounds, [vertex vector], &bounds);
            addV3f(&center, [vertex vector], &center);
        }
        
        scaleV3f(&center, 1.0f / [vertices count], &center);
        
        valid = YES;
    }
}

@end

@implementation VertexData

- (id)init {
    if (self = [super init]) {
        vertices = [[NSMutableArray alloc] init];
        edges = [[NSMutableArray alloc] init];
        sides = [[NSMutableArray alloc] init];
        
        // initialize as huge cube
        Vertex* esb = [[Vertex alloc] initWithX:-4096 y:-4096 z:-4096];
        [vertices addObject:esb];
        [esb release];
        
        Vertex* est = [[Vertex alloc] initWithX:-4096 y:-4096 z:+4096];
        [vertices addObject:est];
        [est release];
        
        Vertex* enb = [[Vertex alloc] initWithX:-4096 y:+4096 z:-4096];
        [vertices addObject:enb];
        [enb release];
        
        Vertex* ent = [[Vertex alloc] initWithX:-4096 y:+4096 z:+4096];
        [vertices addObject:ent];
        [ent release];
        
        Vertex* wsb = [[Vertex alloc] initWithX:+4096 y:-4096 z:-4096];
        [vertices addObject:wsb];
        [wsb release];
        
        Vertex* wst = [[Vertex alloc] initWithX:+4096 y:-4096 z:+4096];
        [vertices addObject:wst];
        [wst release];
        
        Vertex* wnb = [[Vertex alloc] initWithX:+4096 y:+4096 z:-4096];
        [vertices addObject:wnb];
        [wnb release];
        
        Vertex* wnt = [[Vertex alloc] initWithX:+4096 y:+4096 z:+4096];
        [vertices addObject:wnt];
        [wnt release];
        
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
    TPlane* plane = [face boundary];
    
    int keep = 0;
    int drop = 0;
    int undecided = 0;
    
    // mark vertices
    NSEnumerator* vEn = [vertices objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vEn nextObject])) {
        EPointStatus vertexStatus = pointStatus(plane, [vertex vector]);
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

    valid = NO;
    return YES;
}

- (NSArray *)vertices {
    return vertices;
}

- (NSArray *)edges {
    return edges;
}

- (TBoundingBox *)bounds {
    [self validate];
    return &bounds;
}

- (TVector3f *)center {
    [self validate];
    return &center;
}

- (int)edgeCount {
    return [edges count];
}

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList {
    if (isnan(intersectBoundsWithRay([self bounds], theRay)))
        return;
    
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

- (void)dealloc {
    [sides release];
    [edges release];
    [vertices release];
    [super dealloc];
}

@end

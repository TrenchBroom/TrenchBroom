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
#import "Brush.h"
#import "Entity.h"

@interface VertexData (private)

- (void)validate;
- (BOOL)containsPoint:(const TVector3f *)point;
- (EPointStatus)statusOfPoints:(NSArray *)points origin:(TVector3f *)origin direction:(TVector3f *)direction;

@end

@implementation VertexData (private)

- (void)validate {
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

- (BOOL)containsPoint:(const TVector3f *)point {
    NSEnumerator* sideEn = [sides objectEnumerator];
    Side* side;
    while ((side = [sideEn nextObject])) {
        TPlane* plane = [[side face] boundary];
        if (pointStatusFromPlane(plane, point) == PS_ABOVE)
            return NO;
    }
    
    return YES;
}

- (EPointStatus)statusOfPoints:(NSArray *)points origin:(TVector3f *)origin direction:(TVector3f *)direction {
    int above, below;
    
    NSEnumerator* pointEn = [points objectEnumerator];
    Vertex* point;
    while ((point = [pointEn nextObject])) {
        EPointStatus pointStatus =pointStatusFromRay(origin, direction, [point vector]);
        if (pointStatus == PS_ABOVE)
            above++;
        else if (pointStatus == PS_BELOW)
            below++;
        if (above > 0 && below > 0)
            return PS_INSIDE;
    }
    
    return above > 0 ? PS_ABOVE : PS_BELOW;
}

@end

@implementation VertexData

- (id)init {
    if ((self = [super init])) {
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
        Side* southSide = [[Side alloc] initWithEdges:southEdges flipped:southFlipped];
        [sides addObject:southSide];
        [southSide release];
        [southEdges release];
        
        NSArray* northEdges = [[NSArray alloc] initWithObjects:wnbenb, wntwnb, entwnt, enbent, nil];
        BOOL northFlipped[] = {YES, YES, YES, YES};
        Side* northSide = [[Side alloc] initWithEdges:northEdges flipped:northFlipped];
        [sides addObject:northSide];
        [northSide release];
        [northEdges release];

        NSArray* westEdges = [[NSArray alloc] initWithObjects:wsbwnb, wsbwst, wntwst, wntwnb, nil];
        BOOL westFlipped[] = {YES, NO, YES, NO};
        Side* westSide = [[Side alloc] initWithEdges:westEdges flipped:westFlipped];
        [sides addObject:westSide];
        [westSide release];
        [westEdges release];
        
        NSArray* eastEdges = [[NSArray alloc] initWithObjects:enbesb, enbent, estent, estesb, nil];
        BOOL eastFlipped[] = {YES, NO, YES, NO};
        Side* eastSide = [[Side alloc] initWithEdges:eastEdges flipped:eastFlipped];
        [sides addObject:eastSide];
        [eastSide release];
        [eastEdges release];
        
        NSArray* topEdges = [[NSArray alloc] initWithObjects:wstest, estent, entwnt, wntwst, nil];
        BOOL topFlipped[] = {NO, NO, NO, NO};
        Side* topSide = [[Side alloc] initWithEdges:topEdges flipped:topFlipped];
        [sides addObject:topSide];
        [topSide release];
        [topEdges release];
        
        NSArray* bottomEdges = [[NSArray alloc] initWithObjects:esbwsb, wsbwnb, wnbenb, enbesb, nil];
        BOOL bottomFlipped[] = {NO, NO, NO, NO};
        Side* bottomSide = [[Side alloc] initWithEdges:bottomEdges flipped:bottomFlipped];
        [sides addObject:bottomSide];
        [bottomSide release];
        [bottomEdges release];
    }
    
    return self;
}

- (id)initWithFaces:(NSArray *)faces droppedFaces:(NSMutableSet **)droppedFaces {
    if ((self = [self init])) {
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
        EPointStatus vertexStatus = pointStatusFromPlane(plane, [vertex vector]);
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
    
    // create new side from newly created edges
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
    if (!valid)
        [self validate];

    return &bounds;
}

- (TVector3f *)center {
    if (!valid)
        [self validate];

    return &center;
}

- (int)edgeCount {
    return [edges count];
}

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList {
    if (isnan(intersectBoundsWithRay([self bounds], theRay, NULL)))
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

- (void)pickEdgeClosestToRay:(TRay *)theRay maxDistance:(float)theMaxDist hitList:(PickingHitList *)theHitList {
    NSEnumerator* edgeEn = [edges objectEnumerator];
    Edge* edge;
    Edge* closestEdge = nil;
    float closestRayDist;
    float closestDist2 = theMaxDist * theMaxDist + 1;
    while ((edge = [edgeEn nextObject])) {
        float rayDist;
        float dist2 = distanceOfSegmentAndRaySquared([[edge startVertex] vector], [[edge endVertex] vector], theRay, &rayDist);
        if (dist2 < closestDist2) {
            closestRayDist = rayDist;
            closestDist2 = dist2;
            closestEdge = edge;
        }
    }
    
    if (closestDist2 > theMaxDist * theMaxDist)
        return;
    
    TVector3f hitPoint;
    rayPointAtDistance(theRay, closestRayDist, &hitPoint);
    
    PickingHit* hit = [[PickingHit alloc] initWithObject:closestEdge type:HT_CLOSE_EDGE hitPoint:&hitPoint distance:closestRayDist];
    [theHitList addHit:hit];
    [hit release];
}

- (BOOL)intersectsBrush:(id <Brush>)theBrush {
    // separating axis theorem
    // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
    
    NSEnumerator* sideEn = [sides objectEnumerator];
    Side* side;
    while ((side = [sideEn nextObject])) {
        TVector3f* direction = [[side face] norm];
        TVector3f* origin = [[[[side face] vertices] objectAtIndex:0] vector];
        if ([self statusOfPoints:[theBrush vertices] origin:origin direction:direction] == PS_ABOVE)
            return NO;
    }
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        TVector3f* direction = [face norm];
        TVector3f* origin = [[[face vertices] objectAtIndex:0] vector];
        if ([self statusOfPoints:vertices origin:origin direction:direction] == PS_ABOVE)
            return NO;
    }
    
    NSEnumerator* myEdgeEn = [edges objectEnumerator];
    Edge* myEdge;
    while ((myEdge = [myEdgeEn nextObject])) {
        NSEnumerator* theirEdgeEn = [[theBrush edges] objectEnumerator];
        Edge* theirEdge;
        while ((theirEdge = [theirEdgeEn nextObject])) {
            TVector3f myEdgeVec, theirEdgeVec, cross;
            [myEdge asVector:&myEdgeVec];
            [theirEdge asVector:&theirEdgeVec];
            crossV3f(&myEdgeVec, &theirEdgeVec, &cross);
            
            EPointStatus myStatus = [self statusOfPoints:vertices origin:[[myEdge startVertex] vector] direction:&cross];
            if (myStatus != PS_INSIDE) {
                EPointStatus theirStatus = [self statusOfPoints:[theBrush vertices] origin:[[myEdge startVertex] vector] direction:&cross];
                if (theirStatus != PS_INSIDE) {
                    if (myStatus != theirStatus)
                        return NO;
                }
            }
        }
    }
    
    return YES;
}

- (BOOL)containsBrush:(id <Brush>)theBrush {
    NSEnumerator* vertexEn = [[theBrush vertices] objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject]))
        if (![self containsPoint:[vertex vector]])
            return NO;
    
    return YES;
}

- (BOOL)intersectsEntity:(id <Entity>)theEntity {
    // todo this is not entirely correct, what if the brush passes through the entity?
    
    TBoundingBox* entityBounds = [theEntity bounds];

    TVector3f p = entityBounds->min;
    if ([self containsPoint:&p])
        return YES;

    p.x = entityBounds->max.x;
    if ([self containsPoint:&p])
        return YES;

    p.y = entityBounds->max.y;
    if ([self containsPoint:&p])
        return YES;

    p.x = entityBounds->min.x;
    if ([self containsPoint:&p])
        return YES;

    p = entityBounds->max;
    if ([self containsPoint:&p])
        return YES;

    p.x = entityBounds->min.x;
    if ([self containsPoint:&p])
        return YES;
    
    p.y = entityBounds->min.y;
    if ([self containsPoint:&p])
        return YES;

    p.x = entityBounds->max.x;
    if ([self containsPoint:&p])
        return YES;

    return NO;
}

- (BOOL)containsEntity:(id <Entity>)theEntity {
    TBoundingBox* entityBounds = [theEntity bounds];
    
    TVector3f p = entityBounds->min;
    if (![self containsPoint:&p])
        return NO;
    
    p.x = entityBounds->max.x;
    if (![self containsPoint:&p])
        return NO;
    
    p.y = entityBounds->max.y;
    if (![self containsPoint:&p])
        return NO;
    
    p.x = entityBounds->min.x;
    if (![self containsPoint:&p])
        return NO;
    
    p = entityBounds->max;
    if (![self containsPoint:&p])
        return NO;
    
    p.x = entityBounds->min.x;
    if (![self containsPoint:&p])
        return NO;
    
    p.y = entityBounds->min.y;
    if (![self containsPoint:&p])
        return NO;
    
    p.x = entityBounds->max.x;
    if (![self containsPoint:&p])
        return NO;
    
    return YES;
}

- (void)dealloc {
    [sides release];
    [edges release];
    [vertices release];
    [super dealloc];
}

@end

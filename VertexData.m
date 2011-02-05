//
//  VertexData2.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "VertexData.h"
#import "Math.h"
#import "Vertex.h"
#import "Edge.h"
#import "SideEdge.h"
#import "Side.h"
#import "Vector3f.h"
#import "Vector2f.h"
#import "HalfSpace3D.h"
#import "Face.h"
#import "BoundingBox.h"
#import "Plane3D.h"
#import "Ray3D.h"
#import "CoordinatePlane.h"
#import "PickingHit.h"

static NSString* dummy = @"dummy";

@implementation VertexData

- (id)init {
    if (self = [super init]) {
        vertices = [[NSMutableArray alloc] init];
        edges = [[NSMutableArray alloc] init];
        sides = [[NSMutableArray alloc] init];
        faceToSide = [[NSMutableDictionary alloc] init];
        sideToFace = [[NSMutableArray alloc] init];
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
        Side* southSide = [[Side alloc] initWithEdges:southEdges flipped:southFlipped];
        [sides addObject:southSide];
        [sideToFace addObject:dummy];
        [southSide release];
        [southEdges release];
        
        NSArray* northEdges = [[NSArray alloc] initWithObjects:wnbenb, enbent, entwnt, wntwnb, nil];
        BOOL northFlipped[] = {NO, NO, NO, NO};
        Side* northSide = [[Side alloc] initWithEdges:northEdges flipped:northFlipped];
        [sides addObject:northSide];
        [sideToFace addObject:dummy];
        [northSide release];
        [northEdges release];

        NSArray* westEdges = [[NSArray alloc] initWithObjects:wsbwnb, wntwnb, wntwst, wsbwst, nil];
        BOOL westFlipped[] = {NO, YES, NO, YES};
        Side* westSide = [[Side alloc] initWithEdges:westEdges flipped:westFlipped];
        [sides addObject:westSide];
        [sideToFace addObject:dummy];
        [westSide release];
        [westEdges release];
        
        NSArray* eastEdges = [[NSArray alloc] initWithObjects:enbesb, estesb, estent, enbent, nil];
        BOOL eastFlipped[] = {NO, YES, NO, YES};
        Side* eastSide = [[Side alloc] initWithEdges:eastEdges flipped:eastFlipped];
        [sides addObject:eastSide];
        [sideToFace addObject:dummy];
        [eastSide release];
        [eastEdges release];
        
        NSArray* topEdges = [[NSArray alloc] initWithObjects:wstest, wntwst, entwnt, estent, nil];
        BOOL topFlipped[] = {YES, YES, YES, YES};
        Side* topSide = [[Side alloc] initWithEdges:topEdges flipped:topFlipped];
        [sides addObject:topSide];
        [sideToFace addObject:dummy];
        [topSide release];
        [topEdges release];
        
        NSArray* bottomEdges = [[NSArray alloc] initWithObjects:esbwsb, enbesb, wnbenb, wsbwnb, nil];
        BOOL bottomFlipped[] = {YES, YES, YES, YES};
        Side* bottomSide = [[Side alloc] initWithEdges:bottomEdges flipped:bottomFlipped];
        [sides addObject:bottomSide];
        [sideToFace addObject:dummy];
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
        Face* face;
        while ((face = [faceEn nextObject])) {
            if (![self cutWithFace:face droppedFaces:droppedFaces]) {
                [self release];
                return nil;
            }
        }
    }
    
    return self;
}

- (BOOL)cutWithFace:(Face *)face droppedFaces:(NSMutableArray **)droppedFaces {
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
            id face = [sideToFace objectAtIndex:i];
            if (face != dummy)
                [*droppedFaces addObject:face];
            [sideToFace removeObjectAtIndex:i];
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
    Side* side = [[Side alloc] initWithSideEdges:newEdges];
    [sides addObject:side];
    [sideToFace addObject:face];
    [faceToSide setObject:side forKey:[face faceId]];
    [newEdges release];
    
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

- (NSArray *)verticesForFace:(Face *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    Side* side = [faceToSide objectForKey:[face faceId]];
    if (side == nil)
        [NSException raise:NSInvalidArgumentException format:@"no vertex data for face"];
    
    return [side vertices];
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

- (Vector3f *)centerOfFace:(Face *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    
    Side* side = [faceToSide objectForKey:[face faceId]];
    if (side == nil)
        [NSException raise:NSInvalidArgumentException format:@"face %@ does not belong to %@", face, self];
    
    return [side center];
}

- (PickingHit *)pickFace:(Ray3D *)theRay {
    for (int i = 0; i < [sides count]; i++) {
        Side* side = [sides objectAtIndex:i];
        Face* face = [sideToFace objectAtIndex:i];
        
        Vector3f* norm = [face norm];
        float d = [norm dot:[theRay direction]];
        if (!fneg(d))
            continue;
        
        Plane3D* plane = [[face halfSpace] boundary];
        Vector3f* is = [plane intersectWithRay:theRay];
        if (is == nil)
            continue;
        
        CoordinatePlane* cPlane = [CoordinatePlane projectionPlaneForNormal:norm];
        Vector2f* is2D = [cPlane project:is];

        NSArray* sideVertices = [side vertices];
        
        int c = 0;
        Vector3f* v = [sideVertices lastObject];
        Vector2f* v0 = [cPlane project:v];
        [v0 sub:is2D];
        
        NSEnumerator* vertexEn = [sideVertices objectEnumerator];
        while ((v = [vertexEn nextObject])) {
            Vector2f* v1 = [cPlane project:v];
            [v1 sub:is2D];
            
            if ([v0 isNull] || [v1 isNull]) {
                // the point is identical to a polygon vertex, cancel search
                c = 1;
                break;
            }
            
            float x0 = [v0 x];
            float y0 = [v0 y];
            float x1 = [v1 x];
            float y1 = [v1 y];
            
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
            
            v0 = v1;
        }
        
        if (c % 2 == 0)
            continue;
        
        return [PickingHit hitWithObject:face 
                                hitPoint:is 
                                distance:[[Vector3f sub:is subtrahend:[theRay origin]] length]];
    }
    
    return nil;
}

- (void)dealloc {
    [centers release];
    [faceToSide release];
    [sideToFace release];
    [sides release];
    [edges release];
    [vertices release];
    [bounds release];
    [super dealloc];
}

@end

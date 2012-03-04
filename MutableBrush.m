/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "MutableBrush.h"
#import "Face.h"
#import "Brush.h"
#import "MutableFace.h"
#import "Entity.h"
#import "MutableEntity.h"
#import "Camera.h"
#import "IdGenerator.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@interface MutableBrush (private)

- (TVertexData *)vertexData;
- (BOOL)rebuildVertexData:(NSMutableArray **)droppedFaces;

@end

@implementation MutableBrush (private)

- (TVertexData *)vertexData {
    if (!vertexDataValid) {
        initVertexData(&vertexData);
        NSMutableArray* droppedFaces = nil;
        if (!initVertexDataWithFaces(&vertexData, worldBounds, faces, &droppedFaces)) {
            if (droppedFaces != nil) {
                for (MutableFace* droppedFace in droppedFaces) {
                    [droppedFace setBrush:nil];
                    [faces removeObject:droppedFace];
                }
            }
            freeVertexData(&vertexData);
            return NULL;
        } else {
            vertexDataValid = YES;
        }
        
        verticesOnGrid = YES;
        for (int i = 0; i < vertexData.vertices.count && verticesOnGrid; i++)
            verticesOnGrid = intV3f(&vertexData.vertices.items[i]->position);
    }
    
    return &vertexData;
}

- (BOOL)rebuildVertexData:(NSMutableArray **)droppedFaces {
    initVertexData(&vertexData);
    if (!initVertexDataWithFaces(&vertexData, worldBounds, faces, droppedFaces)) {
        freeVertexData(&vertexData);
        return NO;
    }
    
    return YES;
}

@end

@implementation MutableBrush

- (id)init {
    if ((self = [super init])) {
        brushId = [[[IdGenerator sharedGenerator] getId] retain];
        faces = [[NSMutableArray alloc] init];
        initVertexData(&vertexData);
        
        filePosition = -1;
    }
    return self;
}

- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds {
    NSAssert(theWorldBounds != NULL, @"world bounds must not be NULL");

    if ((self = [self init])) {
        worldBounds = theWorldBounds;
    }
    
    return self;
}

- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds brushTemplate:(id <Brush>)theTemplate {
    NSAssert(theTemplate != nil, @"brush template must not be nil");
    
    if ((self = [self initWithWorldBounds:theWorldBounds])) {
        for (id <Face> faceTemplate in [theTemplate faces]) {
            MutableFace* face = [[MutableFace alloc] initWithWorldBounds:worldBounds faceTemplate:faceTemplate];
            [self addFace:face];
            [face release];
        }
    }
    
    return self;
}

- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds brushBounds:(const TBoundingBox *)theBrushBounds texture:(Texture *)theTexture {
    NSAssert(theBrushBounds != NULL, @"brush bounds must not be NULL");
    
    if ((self = [self initWithWorldBounds:theWorldBounds])) {
        TVector3f p1, p2, p3;
        
        p1 = theBrushBounds->min;
        p2 = p1;
        p2.z = theBrushBounds->max.z;
        p3 = p1;
        p3.x = theBrushBounds->max.x;
        MutableFace* frontFace = [[MutableFace alloc] initWithWorldBounds:worldBounds point1:&p1 point2:&p2 point3:&p3];
        [frontFace setTexture:theTexture];
        [self addFace:frontFace];
        [frontFace release];

        p2 = p1;
        p2.y = theBrushBounds->max.y;
        p3 = p1;
        p3.z = theBrushBounds->max.z;
        MutableFace* leftFace = [[MutableFace alloc] initWithWorldBounds:worldBounds point1:&p1 point2:&p2 point3:&p3];
        [leftFace setTexture:theTexture];
        [self addFace:leftFace];
        [leftFace release];

        p2 = p1;
        p2.x = theBrushBounds->max.x;
        p3 = p1;
        p3.y = theBrushBounds->max.y;
        MutableFace* bottomFace = [[MutableFace alloc] initWithWorldBounds:worldBounds point1:&p1 point2:&p2 point3:&p3];
        [bottomFace setTexture:theTexture];
        [self addFace:bottomFace];
        [bottomFace release];

        p1 = theBrushBounds->max;
        p2 = p1;
        p2.x = theBrushBounds->min.x;
        p3 = p1;
        p3.z = theBrushBounds->min.z;
        MutableFace* backFace = [[MutableFace alloc] initWithWorldBounds:worldBounds point1:&p1 point2:&p2 point3:&p3];
        [backFace setTexture:theTexture];
        [self addFace:backFace];
        [backFace release];

        p2 = p1;
        p2.z = theBrushBounds->min.z;
        p3 = p1;
        p3.y = theBrushBounds->min.y;
        MutableFace* rightFace = [[MutableFace alloc] initWithWorldBounds:worldBounds point1:&p1 point2:&p2 point3:&p3];
        [rightFace setTexture:theTexture];
        [self addFace:rightFace];
        [rightFace release];

        p2 = p1;
        p2.y = theBrushBounds->min.y;
        p3 = p1;
        p3.x = theBrushBounds->min.x;
        MutableFace* topFace = [[MutableFace alloc] initWithWorldBounds:worldBounds point1:&p1 point2:&p2 point3:&p3];
        [topFace setTexture:theTexture];
        [self addFace:topFace];
        [topFace release];
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    MutableBrush* result = [[MutableBrush allocWithZone:zone] init];
    [result->brushId release];
    result->brushId = [brushId retain];
    result->worldBounds = worldBounds;
    
    [result setEntity:entity];
    [result setFilePosition:filePosition];

    for (id <Face> face in faces) {
        id <Face> faceCopy = [face copy];
        [result addFace:faceCopy];
        [faceCopy release];
    }
    
    return result;

}

- (void)dealloc {
    [brushId release];
    [faces release];
    freeVertexData(&vertexData);
    [super dealloc];
}

- (BOOL)addFace:(MutableFace *)face {
    NSMutableArray* droppedFaces = nil;
    ECutResult result = cutVertexData([self vertexData], face, &droppedFaces);
    if (result == CR_NULL)
        return NO;
    
    if (result == CR_REDUNDANT)
        return YES;
    
    if (droppedFaces != nil) {
        for (MutableFace* droppedFace in droppedFaces) {
            [droppedFace setBrush:nil];
            [faces removeObjectIdenticalTo:droppedFace];
        }
    }

    [face setBrush:self];
    [faces addObject:face];
    return YES;
}

- (void)setEntity:(MutableEntity *)theEntity {
    entity = theEntity;
}

- (void)translateBy:(const TVector3f *)theDelta lockTextures:(BOOL)lockTextures {
    for (MutableFace* face in faces)
        [face translateBy:theDelta lockTexture:lockTextures];

    if (vertexDataValid)
        translateVertexData(&vertexData, theDelta);
    
    [entity brushChanged:self];
}

- (void)rotate90CW:(EAxis)theAxis center:(const TVector3f *)theCenter lockTextures:(BOOL)lockTextures {
    for (MutableFace* face in faces)
        [face rotate90CW:theAxis center:theCenter lockTexture:lockTextures];

    if (vertexDataValid)
        rotateVertexData90CW(&vertexData, theAxis, theCenter);
    
    [entity brushChanged:self];
}

- (void)rotate90CCW:(EAxis)theAxis center:(const TVector3f *)theCenter lockTextures:(BOOL)lockTextures {
    for (MutableFace* face in faces)
        [face rotate90CCW:theAxis center:theCenter lockTexture:lockTextures];

    if (vertexDataValid)
        rotateVertexData90CCW(&vertexData, theAxis, theCenter);
    
    [entity brushChanged:self];
}

- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter lockTextures:(BOOL)lockTextures {
    for (MutableFace* face in faces)
        [face rotate:theRotation center:theCenter lockTexture:lockTextures];

    if (vertexDataValid)
        rotateVertexData(&vertexData, theRotation, theCenter);
    
    snapVertexData(&vertexData);
    
    [entity brushChanged:self];
}

- (void)flipAxis:(EAxis)theAxis center:(const TVector3f *)theCenter lockTextures:(BOOL)lockTextures {
    for (MutableFace* face in faces)
        [face flipAxis:theAxis center:theCenter lockTexture:lockTextures];

    if (vertexDataValid)
        flipVertexData(&vertexData, theAxis, theCenter);
    
    [entity brushChanged:self];
}

- (void)snap {
    snapVertexData([self vertexData]);
    [entity brushChanged:self];
}

- (void)setSelected:(BOOL)isSelected {
    selected = isSelected;
}

- (BOOL)canDrag:(MutableFace *)face by:(float)dist {
    NSMutableArray* testFaces = [[NSMutableArray alloc] initWithArray:faces];
    [testFaces removeObjectIdenticalTo:face];

    MutableFace* testFace = [[MutableFace alloc] initWithWorldBounds:worldBounds faceTemplate:face];
    [testFace dragBy:dist lockTexture:NO];
    [testFaces addObject:testFace];
    [testFace release];
    
    const TPlane* oldBoundary = [face boundary];
    const TPlane* newBoundary = [testFace boundary];
    
    if (equalPlane(oldBoundary, newBoundary))
        return NO;
    
    NSMutableArray* droppedFaces = nil;
    TVertexData testData;
    
    BOOL canDrag = initVertexDataWithFaces(&testData, worldBounds, testFaces, &droppedFaces) && 
                   (droppedFaces == nil || [droppedFaces count] == 0) && 
                   boundsContainBounds(worldBounds, &testData.bounds);
    
    freeVertexData(&testData);

    if (vertexDataValid) {
        for (int i = 0; i < vertexData.sides.count; i++) {
            TSide* side = vertexData.sides.items[i];
            if (side->face != nil)
                [side->face setSide:side];
        }
    }

    return canDrag;
}

- (void)drag:(MutableFace *)face by:(float)dist lockTexture:(BOOL)lockTexture {
    [face dragBy:dist lockTexture:lockTexture];
    [self invalidateVertexData];
    [entity brushChanged:self];
}

- (void)enlargeBy:(float)delta lockTexture:(BOOL)lockTexture {
    for (MutableFace* face in faces)
        [face dragBy:delta lockTexture:lockTexture];
    [self invalidateVertexData];
    [entity brushChanged:self];
}

- (TDragResult)dragVertex:(int)theVertexIndex by:(const TVector3f *)theDelta {
    NSMutableArray* addedFaces = [[NSMutableArray alloc] init];
    NSMutableArray* removedFaces = [[NSMutableArray alloc] init];
    
    TDragResult result = dragVertex([self vertexData], theVertexIndex, *theDelta, addedFaces, removedFaces);

    for (MutableFace* removedFace in removedFaces) {
        [removedFace setBrush:nil];
        [faces removeObjectIdenticalTo:removedFace];
    }
    
    for (MutableFace* addedFace in addedFaces) {
        [addedFace setBrush:self];
        [faces addObject:addedFace];
    }
    
    [entity brushChanged:self];
    
    [addedFaces release];
    [removedFaces release];
    
    return result;
}

- (TDragResult)dragEdge:(int)theEdgeIndex by:(const TVector3f *)theDelta {
    NSMutableArray* addedFaces = [[NSMutableArray alloc] init];
    NSMutableArray* removedFaces = [[NSMutableArray alloc] init];
    
    TDragResult result = dragEdge([self vertexData], theEdgeIndex, *theDelta, addedFaces, removedFaces);
    
    for (MutableFace* removedFace in removedFaces) {
        [removedFace setBrush:nil];
        [faces removeObjectIdenticalTo:removedFace];
    }
    
    for (MutableFace* addedFace in addedFaces) {
        [addedFace setBrush:self];
        [faces addObject:addedFace];
    }
    
    [entity brushChanged:self];
    
    [addedFaces release];
    [removedFaces release];
    
    return result;
}

- (TDragResult)dragFace:(int)theFaceIndex by:(const TVector3f *)theDelta {
    NSMutableArray* addedFaces = [[NSMutableArray alloc] init];
    NSMutableArray* removedFaces = [[NSMutableArray alloc] init];
    
    TDragResult result = dragSide([self vertexData], theFaceIndex, *theDelta, addedFaces, removedFaces);
    
    for (MutableFace* removedFace in removedFaces) {
        [removedFace setBrush:nil];
        [faces removeObjectIdenticalTo:removedFace];
    }
    
    for (MutableFace* addedFace in addedFaces) {
        [addedFace setBrush:self];
        [faces addObject:addedFace];
    }
    
    [entity brushChanged:self];
    
    [addedFaces release];
    [removedFaces release];
    
    return result;
}

- (void)deleteFace:(MutableFace *)face {
    [faces removeObjectIdenticalTo:face];
    [self invalidateVertexData];
    [entity brushChanged:self];
}

- (BOOL)canDeleteFace:(MutableFace *)face {
    NSMutableArray* testFaces = [[NSMutableArray alloc] initWithArray:faces];
    [testFaces removeObjectIdenticalTo:face];
    
    NSMutableArray* droppedFaces = nil;
    TVertexData testData;

    BOOL canDelete = YES;
    
    initVertexDataWithFaces(&testData, worldBounds, testFaces, &droppedFaces);
    for (int i = 0; i < testData.sides.count && canDelete; i++)
        canDelete = testData.sides.items[i]->face != nil;
    
    freeVertexData(&testData);
    [self invalidateVertexData];
    return canDelete;
}

- (void)invalidateVertexData {
    if (vertexDataValid) {
        freeVertexData(&vertexData);
        vertexDataValid = NO;
    }
}

- (int)filePosition {
    return filePosition;
}

- (void)setFilePosition:(int)theFilePosition {
    filePosition = theFilePosition;
}

- (void)restore:(id <Brush>)theTemplate {
    NSAssert(theTemplate != nil, @"template must not be nil");
    NSAssert([brushId isEqualTo:[theTemplate brushId]], @"brush id must be equal");
    
    [self invalidateVertexData];
    [faces removeAllObjects];
    
    for (id <Face> face in [theTemplate faces]) {
        MutableFace* copy = [face copy];
        [faces addObject:copy];
        [copy setBrush:self];
        [copy release];
    }

    [entity brushChanged:self];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"ID: %i, number of faces: %i", 
            [brushId intValue], 
            [faces count]
            ];
}

# pragma mark -
# pragma mark @implementation Brush

- (NSNumber *)brushId {
    return brushId;
}

- (id <Entity>)entity {
    return entity;
}

- (NSArray *)faces {
    return faces;
}

- (const TVertexList *)vertices {
    return &[self vertexData]->vertices;
}

- (const TEdgeList *)edges {
    return &[self vertexData]->edges;
}

- (BOOL)verticesOnGrid {
    if (!vertexDataValid)
        [self vertexData];
    return verticesOnGrid;
}

- (BOOL)selected {
    return selected;
}

- (void)planePoint1:(TVector3f *)thePoint1 point2:(TVector3f *)thePoint2 point3:(TVector3f *)thePoint3 {
    const TVertexList* vertices = [self vertices];
    if (vertices->count == 3) {
        thePoint1 = &vertices->items[0]->position;
        thePoint2 = &vertices->items[1]->position;
        thePoint3 = &vertices->items[2]->position;
    } else {
        
    }
}

- (const TBoundingBox *)bounds {
    return &[self vertexData]->bounds;
}

- (const TBoundingBox *)worldBounds {
    return worldBounds;
}

- (void)pick:(const TRay *)theRay hitList:(PickingHitList *)theHitList {
    TVertexData* vd = [self vertexData];
    float dist = intersectBoundsWithRay(&vd->bounds, theRay, NULL);
    if (isnan(dist))
        return;

    dist = NAN;
    TVector3f hitPoint;
    TSide* side;
    for (int i = 0; i < vd->sides.count && isnan(dist); i++) {
        side = vd->sides.items[i];
        dist = pickSide(side, theRay, &hitPoint);
    }

    if (!isnan(dist)) {
        PickingHit* faceHit = [[PickingHit alloc] initWithObject:side->face type:HT_FACE hitPoint:&hitPoint distance:dist];
        [theHitList addHit:faceHit];
        [faceHit release];
    }
}

- (void)pickVertices:(const TRay *)theRay handleRadius:(float)theRadius hitList:(PickingHitList *)theHitList {
    TVertexData* vd = [self vertexData];
    TVector3f hitPoint, center, diff;
    
    for (int i = 0; i < vd->vertices.count; i++) {
        TVector3f* vertex = &vd->vertices.items[i]->position;
        
        subV3f(vertex, &theRay->origin, &diff);
        float vertexDist = lengthV3f(&diff);
        
        float dist = intersectSphereWithRay(vertex, theRadius * vertexDist / 300, theRay);
        if (!isnan(dist)) {
            rayPointAtDistance(theRay, dist, &hitPoint);
            PickingHit* hit = [[PickingHit alloc] initWithObject:self vertexIndex:i hitPoint:&hitPoint distance:dist];
            [theHitList addHit:hit];
            [hit release];
        }
    }
    
    for (int i = 0; i < vd->edges.count; i++) {
        TEdge* edge = vd->edges.items[i];
        centerOfEdge(edge, &center);

        subV3f(&center, &theRay->origin, &diff);
        float vertexDist = lengthV3f(&diff);

        float dist = intersectSphereWithRay(&center, theRadius * vertexDist / 300, theRay);
        if (!isnan(dist)) {
            rayPointAtDistance(theRay, dist, &hitPoint);
            PickingHit* hit = [[PickingHit alloc] initWithObject:self edgeIndex:i hitPoint:&hitPoint distance:dist];
            [theHitList addHit:hit];
            [hit release];
        }
    }

    for (int i = 0; i < vd->sides.count; i++) {
        TSide* side = vd->sides.items[i];
        centerOfVertices(&side->vertices, &center);
        
        subV3f(&center, &theRay->origin, &diff);
        float vertexDist = lengthV3f(&diff);
        
        float dist = intersectSphereWithRay(&center, theRadius * vertexDist / 300, theRay);
        if (!isnan(dist)) {
            rayPointAtDistance(theRay, dist, &hitPoint);
            PickingHit* hit = [[PickingHit alloc] initWithObject:self faceIndex:i hitPoint:&hitPoint distance:dist];
            [theHitList addHit:hit];
            [hit release];
        }
    }
}


- (void)pickClosestFace:(const TRay *)theRay maxDistance:(float)theMaxDist hitList:(PickingHitList *)theHitList {
    TVertexData* vd = [self vertexData];

    TEdge* edge;
    TEdge* closestEdge = nil;
    float closestRayDist;
    float closestDist2 = theMaxDist * theMaxDist + 1;
    for (int i = 0; i < vd->edges.count; i++) {
        edge = vd->edges.items[i];
        float rayDist;
        float dist2 = distanceOfSegmentAndRaySquared(&edge->startVertex->position, &edge->endVertex->position, theRay, &rayDist);
        if (dist2 < closestDist2) {
            closestRayDist = rayDist;
            closestDist2 = dist2;
            closestEdge = edge;
        }
    }
    
    if (closestDist2 > theMaxDist * theMaxDist)
        return;

    TVector3f hitPoint;
    float leftDist = pickSide(closestEdge->leftSide, theRay, &hitPoint);
    float rightDist = pickSide(closestEdge->rightSide, theRay, &hitPoint);

    PickingHit* hit;
    if (!isnan(leftDist)) {
        hit = [[PickingHit alloc] initWithObject:closestEdge->leftSide->face type:HT_CLOSE_FACE hitPoint:&hitPoint distance:leftDist];
    } else if (!isnan(rightDist)) {
        hit = [[PickingHit alloc] initWithObject:closestEdge->rightSide->face type:HT_CLOSE_FACE hitPoint:&hitPoint distance:rightDist];
    } else {
        rayPointAtDistance(theRay, closestRayDist, &hitPoint);
        if (dotV3f([closestEdge->leftSide->face norm], &theRay->direction) >= 0) {
            hit = [[PickingHit alloc] initWithObject:closestEdge->leftSide->face type:HT_CLOSE_FACE hitPoint:&hitPoint distance:closestRayDist];
        } else {
            hit = [[PickingHit alloc] initWithObject:closestEdge->rightSide->face type:HT_CLOSE_FACE hitPoint:&hitPoint distance:closestRayDist];
        }
    }
    
    [theHitList addHit:hit];
    [hit release];
}

- (BOOL)intersectsBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    if (!boundsIntersectWithBounds([self bounds], [theBrush bounds]))
        return NO;
    
    // separating axis theorem
    // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
    
    const TVertexList* theirVertices = [theBrush vertices];
    
    for (id <Face> myFace in faces) {
        const TVector3f* origin = &[myFace vertices]->items[0]->position;
        const TVector3f* direction = [myFace norm];
        if (vertexStatusFromRay(origin, direction, theirVertices) == PS_ABOVE)
            return NO;
    }
    
    const TVertexList* myVertices = [self vertices];
    
    for (id <Face> theirFace in [theBrush faces]) {
        TVector3f* origin = &[theirFace vertices]->items[0]->position;
        const TVector3f* direction = [theirFace norm];
        if (vertexStatusFromRay(origin, direction, myVertices) == PS_ABOVE)
            return NO;
    }

    const TEdgeList* myEdges = [self edges];
    const TEdgeList* theirEdges = [theBrush edges];

    for (int i = 0; i < myEdges->count; i++) {
        TEdge* myEdge = myEdges->items[i];
        for (int j = 0; j < theirEdges->count; j++) {
            TEdge* theirEdge = theirEdges->items[j];
            TVector3f myEdgeVec, theirEdgeVec, direction;
            edgeVector(myEdge, &myEdgeVec);
            edgeVector(theirEdge, &theirEdgeVec);
            
            crossV3f(&myEdgeVec, &theirEdgeVec, &direction);
            TVector3f* origin = &myEdge->startVertex->position;
            
            EPointStatus myStatus = vertexStatusFromRay(origin, &direction, myVertices);
            if (myStatus != PS_INSIDE) {
                EPointStatus theirStatus = vertexStatusFromRay(origin, &direction, theirVertices);
                if (theirStatus != PS_INSIDE) {
                    if (myStatus != theirStatus)
                        return NO;
                }
            }
        }
    }
    
    return YES;
}

- (BOOL)containsPoint:(const TVector3f *)thePoint {
    NSAssert(thePoint != NULL, @"point must not be NULL");
    if (!boundsContainPoint([self bounds], thePoint))
        return NO;
    
    return vertexDataContainsPoint([self vertexData], thePoint);
}

- (BOOL)containsBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    if (!boundsContainBounds([self bounds], [theBrush bounds]))
        return NO;

    TVertexData* myVertexData = [self vertexData];
    const TVertexList* theirVertices = [theBrush vertices];
    
    for (int i = 0; i < theirVertices->count; i++)
        if (!vertexDataContainsPoint(myVertexData, &theirVertices->items[i]->position))
            return NO;
    
    return YES;
}

- (BOOL)intersectsEntity:(id <Entity>)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    if (!boundsIntersectWithBounds([self bounds], [theEntity bounds]))
        return NO;
    
    const TBoundingBox* entityBounds = [theEntity bounds];
    TVertexData* myVertexData = [self vertexData];
    
    TVector3f p = entityBounds->min;
    if (vertexDataContainsPoint(myVertexData, &p))
        return YES;
    
    p.x = entityBounds->max.x;
    if (vertexDataContainsPoint(myVertexData, &p))
        return YES;
    
    p.y = entityBounds->max.y;
    if (vertexDataContainsPoint(myVertexData, &p))
        return YES;
    
    p.x = entityBounds->min.x;
    if (vertexDataContainsPoint(myVertexData, &p))
        return YES;
    
    p = entityBounds->max;
    if (vertexDataContainsPoint(myVertexData, &p))
        return YES;
    
    p.x = entityBounds->min.x;
    if (vertexDataContainsPoint(myVertexData, &p))
        return YES;
    
    p.y = entityBounds->min.y;
    if (vertexDataContainsPoint(myVertexData, &p))
        return YES;
    
    p.x = entityBounds->max.x;
    if (vertexDataContainsPoint(myVertexData, &p))
        return YES;
    
    return NO;
}

- (BOOL)containsEntity:(id <Entity>)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    if (!boundsContainBounds([self bounds], [theEntity bounds]))
        return NO;
    
    const TBoundingBox* entityBounds = [theEntity bounds];
    TVertexData* myVertexData = [self vertexData];
    
    TVector3f p = entityBounds->min;
    if (!vertexDataContainsPoint(myVertexData, &p))
        return NO;
    
    p.x = entityBounds->max.x;
    if (!vertexDataContainsPoint(myVertexData, &p))
        return NO;
    
    p.y = entityBounds->max.y;
    if (!vertexDataContainsPoint(myVertexData, &p))
        return NO;
    
    p.x = entityBounds->min.x;
    if (!vertexDataContainsPoint(myVertexData, &p))
        return NO;
    
    p = entityBounds->max;
    if (!vertexDataContainsPoint(myVertexData, &p))
        return NO;
    
    p.x = entityBounds->min.x;
    if (!vertexDataContainsPoint(myVertexData, &p))
        return NO;
    
    p.y = entityBounds->min.y;
    if (!vertexDataContainsPoint(myVertexData, &p))
        return NO;
    
    p.x = entityBounds->max.x;
    if (!vertexDataContainsPoint(myVertexData, &p))
        return NO;
    
    return YES;
}

@end
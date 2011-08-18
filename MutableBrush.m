//
//  Brush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MutableBrush.h"
#import "Face.h"
#import "Brush.h"
#import "MutableFace.h"
#import "Entity.h"
#import "MutableEntity.h"
#import "IdGenerator.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@interface MutableBrush (private)

- (TVertexData *)vertexData;

@end

@implementation MutableBrush (private)

- (TVertexData *)vertexData {
    if (!vertexDataValid) {
        initVertexData(&vertexData);
        NSMutableArray* droppedFaces = nil;
        if (!initVertexDataWithFaces(&vertexData, worldBounds, faces, &droppedFaces)) {
            if (droppedFaces != nil) {
                NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
                MutableFace* droppedFace;
                while ((droppedFace = [droppedFacesEn nextObject])) {
                    [droppedFace setBrush:nil];
                    [faces removeObject:droppedFace];
                }
            }
            freeVertexData(&vertexData);
            return NULL;
        } else {
            vertexDataValid = YES;
        }
    }
    
    return &vertexData;
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

- (id)initWithWorldBounds:(TBoundingBox *)theWorldBounds {
    NSAssert(theWorldBounds != NULL, @"world bounds must not be NULL");

    if ((self = [self init])) {
        worldBounds = theWorldBounds;
    }
    
    return self;
}

- (id)initWithWorldBounds:(TBoundingBox *)theWorldBounds brushTemplate:(id <Brush>)theTemplate {
    NSAssert(theTemplate != nil, @"brush template must not be nil");
    
    if ((self = [self initWithWorldBounds:theWorldBounds])) {
        NSEnumerator* faceEn = [[theTemplate faces] objectEnumerator];
        id <Face> faceTemplate;
        while ((faceTemplate = [faceEn nextObject])) {
            MutableFace* face = [[MutableFace alloc] initWithFaceTemplate:faceTemplate];
            [self addFace:face];
            [face release];
        }
    }
    
    return self;
}

- (id)initWithWorldBounds:(TBoundingBox *)theWorldBounds brushBounds:(TBoundingBox *)theBrushBounds texture:(NSString *)theTexture {
    NSAssert(theBrushBounds != NULL, @"brush bounds must not be NULL");
    
    if ((self = [self initWithWorldBounds:theWorldBounds])) {
        TVector3i min, max, p1, p2, p3;
        
        roundV3f(&theBrushBounds->min, &min);
        roundV3f(&theBrushBounds->max, &max);
        
        p1 = min;
        p2 = min;
        p2.z = max.z;
        p3 = min;
        p3.x = max.x;
        MutableFace* frontFace = [[MutableFace alloc] initWithPoint1:&p1 point2:&p2 point3:&p3 texture:theTexture];
        [self addFace:frontFace];
        [frontFace release];
        
        p1 = min;
        p2 = min;
        p2.y = max.y;
        p3 = min;
        p3.z = max.z;
        MutableFace* leftFace = [[MutableFace alloc] initWithPoint1:&p1 point2:&p2 point3:&p3 texture:theTexture];
        [self addFace:leftFace];
        [leftFace release];
        
        p1 = min;
        p2 = min;
        p2.x = max.x;
        p3 = min;
        p3.y = max.y;
        MutableFace* bottomFace = [[MutableFace alloc] initWithPoint1:&p1 point2:&p2 point3:&p3 texture:theTexture];
        [self addFace:bottomFace];
        [bottomFace release];
        
        p1 = max;
        p2 = max;
        p2.x = min.x;
        p3 = max;
        p3.z = min.z;
        MutableFace* backFace = [[MutableFace alloc] initWithPoint1:&p1 point2:&p2 point3:&p3 texture:theTexture];
        [self addFace:backFace];
        [backFace release];
        
        p1 = max;
        p2 = max;
        p2.z = min.z;
        p3 = max;
        p3.y = min.y;
        MutableFace* rightFace = [[MutableFace alloc] initWithPoint1:&p1 point2:&p2 point3:&p3 texture:theTexture];
        [self addFace:rightFace];
        [rightFace release];
        
        p1 = max;
        p2 = max;
        p2.y = min.y;
        p3 = max;
        p3.x = min.x;
        MutableFace* topFace = [[MutableFace alloc] initWithPoint1:&p1 point2:&p2 point3:&p3 texture:theTexture];
        [self addFace:topFace];
        [topFace release];
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone {
    MutableBrush* result = [[MutableBrush allocWithZone:zone] init];
    [result->brushId release];
    result->brushId = [brushId retain];
    
    [result setEntity:entity];
    [result setFilePosition:filePosition];

    NSEnumerator* faceEn = [faces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
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
    if (!cutVertexData([self vertexData], face, &droppedFaces))
        return NO;
    
    if (droppedFaces != nil) {
        NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
        MutableFace* droppedFace;
        while ((droppedFace = [droppedFacesEn nextObject])) {
            [droppedFace setBrush:nil];
            [faces removeObject:droppedFace];
        }
    }

    [face setBrush:self];
    [faces addObject:face];
    return YES;
}

- (void)removeFace:(MutableFace *)face {
    [face setBrush:nil];
    [faces removeObject:face];
    if (vertexDataValid) {
        freeVertexData(&vertexData);
        vertexDataValid = NO;
    }
}

- (void)setEntity:(MutableEntity *)theEntity {
    entity = theEntity;
}

- (void)translateBy:(TVector3i *)theDelta {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face translateBy:theDelta];

    if (vertexDataValid) {
        TVector3f deltaf;
        setV3f(&deltaf, theDelta);
        translateVertexData(&vertexData, &deltaf);
    }
}

- (void)rotateZ90CW:(TVector3i *)theCenter {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face rotateZ90CW:theCenter];

    if (vertexDataValid) {
        TVector3f centerf;
        setV3f(&centerf, theCenter);
        rotateVertexDataZ90CW(&vertexData, &centerf);
    }
}

- (void)rotateZ90CCW:(TVector3i *)theCenter {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face rotateZ90CCW:theCenter];

    if (vertexDataValid) {
        TVector3f centerf;
        setV3f(&centerf, theCenter);
        rotateVertexDataZ90CCW(&vertexData, &centerf);
    }
}

- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face rotate:theRotation center:theCenter];

    if (vertexDataValid) {
        freeVertexData(&vertexData);
        vertexDataValid = NO;
    }
}

- (void)drag:(MutableFace *)face by:(float)dist {
    [face dragBy:dist];

    if (vertexDataValid) {
        freeVertexData(&vertexData);
        vertexDataValid = NO;
    }
}

- (BOOL)canDrag:(MutableFace *)face by:(float)dist {
    NSMutableArray* testFaces = [[NSMutableArray alloc] initWithArray:faces];
    [testFaces removeObject:face];

    MutableFace* testFace = [[MutableFace alloc] initWithFaceTemplate:face];
    [testFace dragBy:dist];
    [testFaces addObject:testFace];
    [testFace release];
    
    NSMutableArray* droppedFaces = nil;
    TVertexData testData;
    initVertexData(&testData);
    
    BOOL canDrag = initVertexDataWithFaces(&testData, worldBounds, testFaces, &droppedFaces) && 
                   (droppedFaces == nil || [droppedFaces count] == 0) && 
                   boundsContainBounds(worldBounds, vertexDataBounds(&testData));
    
    freeVertexData(&testData);
    return canDrag;
    
}

- (int)filePosition {
    return filePosition;
}

- (void)setFilePosition:(int)theFilePosition {
    filePosition = theFilePosition;
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

- (TVertex **)vertices {
    return [self vertexData]->vertices;
}

- (int)vertexCount {
    return [self vertexData]->vertexCount;
}

- (TEdge **)edges {
    return [self vertexData]->edges;
}

- (int)edgeCount {
    return [self vertexData]->edgeCount;
}

- (const TBoundingBox *)bounds {
    return vertexDataBounds([self vertexData]);
}

- (const TVector3f *)center {
    return vertexDataCenter([self vertexData]);
}

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList {
    TVertexData* vd = [self vertexData];
    if (isnan(intersectBoundsWithRay(vertexDataBounds(vd), theRay, NULL)))
        return;

    float dist = NAN;
    TVector3f hitPoint;
    TSide* side;
    for (int i = 0; i < vd->sideCount && isnan(dist); i++) {
        side = vd->sides[i];
        dist = pickSide(side, theRay, &hitPoint);
    }

    if (!isnan(dist)) {
        PickingHit* faceHit = [[PickingHit alloc] initWithObject:side->face type:HT_FACE hitPoint:&hitPoint distance:dist];
        PickingHit* brushHit = [[PickingHit alloc] initWithObject:[side->face brush] type:HT_BRUSH hitPoint:&hitPoint distance:dist];
        [theHitList addHit:faceHit];
        [theHitList addHit:brushHit];
        [faceHit release];
        [brushHit release];
    }
}

- (void)pickFace:(TRay *)theRay maxDistance:(float)theMaxDist hitList:(PickingHitList *)theHitList {
    TVertexData* vd = [self vertexData];

    TEdge* edge;
    TEdge* closestEdge = nil;
    float closestRayDist;
    float closestDist2 = theMaxDist * theMaxDist + 1;
    for (int i = 0; i < vd->edgeCount; i++) {
        edge = vd->edges[i];
        float rayDist;
        float dist2 = distanceOfSegmentAndRaySquared(&edge->startVertex->vector, &edge->endVertex->vector, theRay, &rayDist);
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
        hit = [[PickingHit alloc] initWithObject:closestEdge->leftSide->face type:HT_CLOSE_EDGE hitPoint:&hitPoint distance:leftDist];
    } else if (!isnan(rightDist)) {
        hit = [[PickingHit alloc] initWithObject:closestEdge->rightSide->face type:HT_CLOSE_EDGE hitPoint:&hitPoint distance:rightDist];
    } else {
        rayPointAtDistance(theRay, closestRayDist, &hitPoint);
        if (dotV3f([closestEdge->leftSide->face norm], &theRay->direction) >= 0) {
            hit = [[PickingHit alloc] initWithObject:closestEdge->leftSide->face type:HT_CLOSE_EDGE hitPoint:&hitPoint distance:closestRayDist];
        } else {
            hit = [[PickingHit alloc] initWithObject:closestEdge->rightSide->face type:HT_CLOSE_EDGE hitPoint:&hitPoint distance:closestRayDist];
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
    
    TVertex** theirVertices = [theBrush vertices];
    int theirVertexCount = [theBrush vertexCount];
    
    NSEnumerator* myFaceEn = [faces objectEnumerator];
    id <Face> myFace;
    while ((myFace = [myFaceEn nextObject])) {
        const TVector3f* origin = &[myFace vertices][0]->vector;
        const TVector3f* direction = [myFace norm];
        if (vertexStatusFromRay(origin, direction, theirVertices, theirVertexCount) == PS_ABOVE)
            return NO;
    }
    
    TVertex** myVertices = [self vertices];
    int myVertexCount = [self vertexCount];
    
    NSEnumerator* theirFaceEn = [[theBrush faces] objectEnumerator];
    id <Face> theirFace;
    while ((theirFace = [theirFaceEn nextObject])) {
        TVector3f* origin = &[theirFace vertices][0]->vector;
        const TVector3f* direction = [theirFace norm];
        if (vertexStatusFromRay(origin, direction, myVertices, myVertexCount) == PS_ABOVE)
            return NO;
    }

    TEdge** myEdges = [self edges];
    int myEdgeCount = [self edgeCount];
    
    TEdge** theirEdges = [theBrush edges];
    int theirEdgeCount = [theBrush edgeCount];

    for (int i = 0; i < myEdgeCount; i++) {
        TEdge* myEdge = myEdges[i];
        for (int j = 0; j < theirEdgeCount; j++) {
            TEdge* theirEdge = theirEdges[j];
            TVector3f myEdgeVec, theirEdgeVec, direction;
            edgeVector(myEdge, &myEdgeVec);
            edgeVector(theirEdge, &theirEdgeVec);
            
            crossV3f(&myEdgeVec, &theirEdgeVec, &direction);
            TVector3f* origin = &myEdge->startVertex->vector;
            
            EPointStatus myStatus = vertexStatusFromRay(origin, &direction, myVertices, myVertexCount);
            if (myStatus != PS_INSIDE) {
                EPointStatus theirStatus = vertexStatusFromRay(origin, &direction, theirVertices, theirVertexCount);
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
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    if (!boundsContainBounds([self bounds], [theBrush bounds]))
        return NO;

    TVertexData* myVertexData = [self vertexData];
    TVertex** theirVertices = [theBrush vertices];
    int theirVertexCount = [theBrush vertexCount];
    
    for (int i = 0; i < theirVertexCount; i++)
        if (!vertexDataContainsPoint(myVertexData, &theirVertices[i]->vector))
            return NO;
    
    return YES;
}

- (BOOL)intersectsEntity:(id <Entity>)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    if (!boundsIntersectWithBounds([self bounds], [theEntity bounds]))
        return NO;
    
    TBoundingBox* entityBounds = [theEntity bounds];
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
    
    TBoundingBox* entityBounds = [theEntity bounds];
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
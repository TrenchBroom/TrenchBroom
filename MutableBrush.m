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
#import "VertexData.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@interface MutableBrush (private)

- (VertexData *)vertexData;

@end

@implementation MutableBrush (private)

- (VertexData *)vertexData {
    if (vertexData == nil) {
        NSMutableSet* droppedFaces = nil;
        vertexData = [[VertexData alloc] initWithFaces:faces droppedFaces:&droppedFaces];
        if (droppedFaces != nil) {
            NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
            MutableFace* droppedFace;
            while ((droppedFace = [droppedFacesEn nextObject])) {
                [droppedFace setBrush:nil];
                [faces removeObject:droppedFace];
            }
        }
    }
    
    return vertexData;
}

@end

@implementation MutableBrush

- (id)init {
    if ((self = [super init])) {
        brushId = [[[IdGenerator sharedGenerator] getId] retain];
        faces = [[NSMutableArray alloc] init];
        vertexData = [[VertexData alloc] init];
        
        flatColor[0] = 0.2f;
        flatColor[1] = 0.2f;
        flatColor[2] = 0.2f;
        
        filePosition = -1;
    }
    
    return self;
}

- (id)initWithBrushTemplate:(id <Brush>)theTemplate {
    if ((self = [self init])) {
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

- (id)copyWithZone:(NSZone *)zone {
    MutableBrush* result = [[MutableBrush allocWithZone:zone] init];
    [result->brushId release];
    result->brushId = [brushId retain];
    for (int i = 0; i < 3; i++)
        result->flatColor[i] = flatColor[i];
    
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
    [vertexData release];
    [faces release];
    [super dealloc];
}

- (BOOL)addFace:(MutableFace *)face {
    NSMutableSet* droppedFaces = nil;
    if (![[self vertexData] cutWithFace:face droppedFaces:&droppedFaces])
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
    [vertexData release];
    vertexData = nil;
}

- (void)setEntity:(MutableEntity *)theEntity {
    entity = theEntity;
}

- (void)translateBy:(TVector3i *)theDelta {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face translateBy:theDelta];
}

- (void)rotateZ90CW:(TVector3i *)theCenter {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face rotateZ90CW:theCenter];
}

- (void)rotateZ90CCW:(TVector3i *)theCenter {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face rotateZ90CCW:theCenter];
}

- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face rotate:theRotation center:theCenter];
}

- (void)faceGeometryChanged:(MutableFace *)face {
    [vertexData release];
    vertexData = nil;
    [entity brushChanged:self];
}

- (BOOL)canDrag:(MutableFace *)face by:(float)dist {
    NSMutableArray* testFaces = [[NSMutableArray alloc] initWithArray:faces];
    [testFaces removeObject:face];

    MutableFace* testFace = [[MutableFace alloc] initWithFaceTemplate:face];
    [testFace dragBy:dist];
    [testFaces addObject:testFace];
    [testFace release];
    
    NSMutableSet* droppedFaces = nil;
    VertexData* testData = [[VertexData alloc] initWithFaces:testFaces droppedFaces:&droppedFaces];
    BOOL canDrag = testData != nil && (droppedFaces == nil || [droppedFaces count] == 0);
    
    [testFaces release];
    [testData release];
    
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

- (NSArray *)vertices {
    return [[self vertexData] vertices];
}

- (NSArray *)edges {
    return [[self vertexData] edges];
}

- (float *)flatColor {
    return flatColor;
}

- (TBoundingBox *)bounds {
    return [[self vertexData] bounds];
}

- (TVector3f *)center {
    return [[self vertexData] center];
}

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList {
    [[self vertexData] pick:theRay hitList:theHitList];
}

- (float)pickHotFace:(TRay *)theRay maxDistance:(float)theMaxDistance hit:(id <Face> *)theHit {
    return [[self vertexData] pickHotFace:theRay maxDistance:theMaxDistance hit:theHit];
}

@end

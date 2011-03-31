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
#import "Vector3f.h"
#import "Vector3i.h"
#import "Quaternion.h"
#import "HalfSpace3D.h"
#import "VertexData.h"
#import "BoundingBox.h"
#import "Ray3D.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@implementation MutableBrush

- (id)init {
    if (self = [super init]) {
        brushId = [[[IdGenerator sharedGenerator] getId] retain];
        faces = [[NSMutableArray alloc] init];
        vertexData = [[VertexData alloc] init];
        
        flatColor[0] = (rand() % 255) / 255.0f;
        flatColor[1] = (rand() % 255) / 255.0f;
        flatColor[2] = (rand() % 255) / 255.0f;
    }
    
    return self;
}

- (id)initWithBrushTemplate:(id <Brush>)theTemplate {
    if (self = [self init]) {
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

- (VertexData *)vertexData {
    if (vertexData == nil) {
        NSMutableArray* droppedFaces = nil;
        vertexData = [[VertexData alloc] initWithFaces:faces droppedFaces:&droppedFaces];
        if (droppedFaces != nil) {
            NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
            MutableFace* droppedFace;
            while ((droppedFace = [droppedFacesEn nextObject]))
                [self removeFace:droppedFace];
        }
    }
    
    return vertexData;
}

- (BOOL)addFace:(MutableFace *)face {
    NSMutableArray* droppedFaces = nil;
    if (![[self vertexData] cutWithFace:face droppedFaces:&droppedFaces])
        return NO;
    
    if (droppedFaces != nil) {
        NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
        MutableFace* droppedFace;
        while ((droppedFace = [droppedFacesEn nextObject]))
            [self removeFace:droppedFace];
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

- (BoundingBox *)bounds {
    return [[self vertexData] bounds];
}

- (Vector3f *)center {
    return [[self vertexData] center];
}


- (void)pickBrush:(Ray3D *)theRay hitList:(PickingHitList *)theHitList {
    [[self vertexData] pickBrush:theRay hitList:theHitList];
}

- (void)pickFace:(Ray3D *)theRay hitList:(PickingHitList *)theHitList {
    [[self vertexData] pickFace:theRay hitList:theHitList];
}

- (void)pickEdge:(Ray3D *)theRay hitList:(PickingHitList *)theHitList {
    [[self vertexData] pickEdge:theRay hitList:theHitList];
}

- (void)pickVertex:(Ray3D *)theRay hitList:(PickingHitList *)theHitList {
    [[self vertexData] pickVertex:theRay hitList:theHitList];
}

- (BoundingBox *)pickingBounds {
    return [[self vertexData] pickingBounds];
}

- (void)setEntity:(MutableEntity *)theEntity {
    entity = theEntity;
}

- (void)translateBy:(Vector3i *)theDelta {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face translateBy:theDelta];
}

- (void)rotateZ90CW:(Vector3i *)theCenter {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face rotateZ90CW:theCenter];
}

- (void)rotateZ90CCW:(Vector3i *)theCenter {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject]))
        [face rotateZ90CCW:theCenter];
}

- (void)faceGeometryChanged:(MutableFace *)face {
    [vertexData release];
    vertexData = nil;
}

- (void)dealloc {
    [brushId release];
    [vertexData release];
    [faces release];
    [super dealloc];
}

@end

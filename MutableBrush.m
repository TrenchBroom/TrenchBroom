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
#import "HalfSpace3D.h"
#import "VertexData.h"
#import "BoundingBox.h"
#import "Ray3D.h"
#import "PickingHit.h"

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

- (id)initWithTemplate:(id <Brush>)theTemplate {
    if (self = [self init]) {
        NSEnumerator* faceEn = [[theTemplate faces] objectEnumerator];
        id <Face> faceTemplate;
        while ((faceTemplate = [faceEn nextObject])) {
            MutableFace* face = [[MutableFace alloc] initWithTemplate:faceTemplate];
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
            while ((droppedFace = [droppedFacesEn nextObject])) {
                NSLog(@"Face %@ was cut away", droppedFace);
                [self removeFace:droppedFace];
            }
        }
    }
    
    return vertexData;
}

- (BOOL)addFace:(MutableFace *)face {
    NSMutableArray* droppedFaces = nil;
    if (![[self vertexData] cutWithFace:face droppedFaces:&droppedFaces]) {
        NSLog(@"Brush %@ was cut away by face %@", self, face);
        return NO;
    }
    
    if (droppedFaces != nil) {
        NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
        MutableFace* droppedFace;
        while ((droppedFace = [droppedFacesEn nextObject])) {
            NSLog(@"Face %@ was cut away by face %@", droppedFace, face);
            [self removeFace:droppedFace];
        }
    }

    [face setBrush:self];
    [faces addObject:face];
    [vertexData release];
    vertexData = nil;
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

- (NSArray *)verticesForFace:(MutableFace *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];

    return [[self vertexData] verticesForFace:face];
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


- (Vector3f *)centerOfFace:(MutableFace *)face {
    return [[self vertexData] centerOfFace:face];
}

- (PickingHit *)pickFace:(Ray3D *)theRay; {
    NSEnumerator* faceEn = [faces objectEnumerator];
    MutableFace* face;
    while ((face = [faceEn nextObject])) {
        PickingHit* hit = [vertexData pickFace:face withRay:theRay];
        if (hit != nil)
            return hit;
    }
    
    return nil;
}

- (PickingHit *)pickFace:(MutableFace *)theFace withRay:(Ray3D *)theRay {
    return [vertexData pickFace:theFace withRay:theRay];
}

- (NSArray *)gridForFace:(MutableFace *)theFace gridSize:(int)gridSize {
    return [vertexData gridForFace:theFace gridSize:gridSize];
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

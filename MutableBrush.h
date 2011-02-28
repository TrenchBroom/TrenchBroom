//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Brush.h"

@class MutableEntity;
@class MutableFace;
@class Vector3i;
@class Vector3f;
@class Face;
@class VertexData;
@class BoundingBox;
@class Ray3D;
@class PickingHit;

@interface MutableBrush : NSObject <Brush> {
    @private
    NSNumber* brushId;
    MutableEntity* entity;
	NSMutableArray* faces;
    VertexData* vertexData;
    float flatColor[3];
}

- (id)initWithTemplate:(id <Brush>)theTemplate;

- (BOOL)addFace:(MutableFace *)face;
- (void)removeFace:(MutableFace *)face;

- (NSNumber* )brushId;

- (NSArray *)faces;

- (Vector3f *)centerOfFace:(MutableFace *)face;
- (PickingHit *)pickFace:(MutableFace *)theFace withRay:(Ray3D *)theRay;
- (NSArray *)gridForFace:(MutableFace *)theFace gridSize:(int)gridSize;
- (NSArray *)verticesForFace:(MutableFace *)face;

- (void)setEntity:(MutableEntity *)theEntity;
- (void)translateBy:(Vector3i *)theDelta;
- (void)faceGeometryChanged:(MutableFace *)face;
@end

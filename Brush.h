//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Entity;
@class Vector3i;
@class Vector3f;
@class Face;
@class VertexData;
@class BoundingBox;
@class Ray3D;
@class PickingHit;

@interface Brush : NSObject {
    @private
    Entity* entity;
    NSNumber* brushId;
	NSMutableArray* faces;
    VertexData* vertexData;
    float flatColor[3];
}

- (Face *)createFaceWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 texture:(NSString *)texture;
- (Face *)createFaceFromTemplate:(Face *)theTemplate;
- (BOOL)addFace:(Face *)face;
- (void)removeFace:(Face *)face;

- (id)initInEntity:(Entity *)theEntity;

- (NSNumber* )brushId;
- (Entity *)entity;

- (NSArray *)faces;
- (NSArray *)verticesForFace:(Face *)face;

- (float *)flatColor;
- (BoundingBox *)bounds;
- (Vector3f *)center;
- (Vector3f *)centerOfFace:(Face *)face;
- (PickingHit *)pickFace:(Ray3D *)theRay;
- (PickingHit *)pickFace:(Face *)theFace withRay:(Ray3D *)theRay;
- (NSArray *)gridForFace:(Face *)theFace gridSize:(int)gridSize;

- (void)translateBy:(Vector3i *)theDelta;

- (void)faceFlagsChanged:(Face *)face;
- (void)faceTextureChanged:(Face *)face oldTexture:(NSString *)oldTexture newTexture:(NSString *)newTexture;
- (void)faceGeometryChanged:(Face *)face;
@end

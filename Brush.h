//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const BrushFaceAdded;
extern NSString* const BrushFaceRemoved;
extern NSString* const BrushFaceGeometryChanged;
extern NSString* const BrushFacePropertiesChanged;
extern NSString* const BrushFaceKey;

@class Entity;
@class Vector3i;
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

- (id)initInEntity:(Entity *)theEntity;

- (NSNumber* )brushId;
- (Entity *)entity;

- (NSArray *)faces;
- (NSArray *)verticesForFace:(Face *)face;
- (int)edgeCount;
- (NSArray *)verticesForWireframe;

- (float *)flatColor;
- (BoundingBox *)bounds;
- (PickingHit *)pickFace:(Ray3D *)theRay;

- (BOOL)postNotifications;
@end

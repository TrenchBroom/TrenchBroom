//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const BrushGeometryChanged;
extern NSString* const BrushFlagsChanged;

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
- (BOOL)addFace:(Face *)face;
- (void)removeFace:(Face *)face;

- (id)initInEntity:(Entity *)theEntity;

- (NSNumber* )brushId;
- (Entity *)entity;

- (NSArray *)faces;
- (NSArray *)verticesForFace:(Face *)face;

- (float *)flatColor;
- (BoundingBox *)bounds;

/*!
    @function
    @abstract   Returns the center of the given face.
    @param      The face.
    @result     The center of the given face.
*/
- (Vector3f *)centerOfFace:(Face *)face;
- (PickingHit *)pickFace:(Ray3D *)theRay;

- (void)translateBy:(Vector3i *)theDelta;

- (BOOL)postNotifications;

/*!
    @function
    @abstract   Returns the undo manager for this brush.
    @discussion This method simply returns the undo manager of the entity to which this brush belongs.
    @result     The undo manager for this brush or nil if there is no undo manager.
*/
- (NSUndoManager *)undoManager;

- (void)faceFlagsChanged:(Face *)theFace;
@end

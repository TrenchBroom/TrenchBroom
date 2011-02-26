//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Map.h"

extern NSString* const FaceAdded;
extern NSString* const FaceRemoved;
extern NSString* const FaceFlagsChanged;
extern NSString* const FaceTextureChanged;
extern NSString* const FaceGeometryChanged;
extern NSString* const FaceKey;
extern NSString* const FaceOldTextureKey;
extern NSString* const FaceNewTextureKey;

extern NSString* const BrushAdded;
extern NSString* const BrushRemoved;
extern NSString* const BrushChanged;
extern NSString* const BrushKey;

extern NSString* const EntityAdded;
extern NSString* const EntityRemoved;
extern NSString* const EntityKey;

extern NSString* const PropertyAdded;
extern NSString* const PropertyRemoved;
extern NSString* const PropertyChanged;
extern NSString* const PropertyKeyKey;
extern NSString* const PropertyOldValueKey;
extern NSString* const PropertyNewValueKey;

@class Entity;
@class Brush;
@class Face;
@class Picker;
@class GLResources;
@class Vector3i;

@interface MapDocument : NSDocument <Map> {
    @private
    NSMutableArray* entities;
    Entity* worldspawn;
    int worldSize;
    BOOL postNotifications;
    Picker* picker;
    GLResources* glResources;
}

- (void)setFace:(Face *)face xOffset:(int)xOffset;
- (void)setFace:(Face *)face yOffset:(int)yOffset;
- (void)translateFaceOffset:(Face *)face xDelta:(int)xDelta yDelta:(int)yDelta;
- (void)setFace:(Face *)face xScale:(float)xScale;
- (void)setFace:(Face *)face yScale:(float)yScale;
- (void)setFace:(Face *)face rotation:(float)angle;
- (void)setFace:(Face *)face texture:(NSString *)texture;
- (void)setFace:(Face *)face point1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3;
- (void)translateFace:(Face *)face xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta;

- (Brush *)createBrushInEntity:(Entity *)entity fromTemplate:(Brush *)theTemplate;
- (void)deleteBrush:(Brush *)brush;
- (void)translateBrush:(Brush *)brush xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta;

- (int)worldSize;

- (void)setPostNotifications:(BOOL)value;

- (Picker *)picker;
- (GLResources *)glResources;

@end

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
extern NSString* const BrushOldBoundsKey;
extern NSString* const BrushNewBoundsKey;

extern NSString* const EntityAdded;
extern NSString* const EntityRemoved;
extern NSString* const EntityKey;

extern NSString* const PropertyAdded;
extern NSString* const PropertyRemoved;
extern NSString* const PropertyChanged;
extern NSString* const PropertyKeyKey;
extern NSString* const PropertyOldValueKey;
extern NSString* const PropertyNewValueKey;

@class MutableEntity;
@class Picker;
@class GLResources;
@class Vector3i;
@protocol Entity;
@protocol Brush;
@protocol Face;

@interface MapDocument : NSDocument <Map> {
    @private
    NSMutableArray* entities;
    MutableEntity* worldspawn;
    int worldSize;
    BOOL postNotifications;
    Picker* picker;
    GLResources* glResources;
}

- (void)setFace:(id <Face>)face xOffset:(int)xOffset;
- (void)setFace:(id <Face>)face yOffset:(int)yOffset;
- (void)translateFaceOffset:(id <Face>)face xDelta:(int)xDelta yDelta:(int)yDelta;
- (void)setFace:(id <Face>)face xScale:(float)xScale;
- (void)setFace:(id <Face>)face yScale:(float)yScale;
- (void)setFace:(id <Face>)face rotation:(float)angle;
- (void)setFace:(id <Face>)face texture:(NSString *)texture;

- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate;
- (void)translateBrush:(id <Brush>)brush xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta;
- (void)deleteBrush:(id <Brush>)brush;

- (id <Entity>)createEntity;

- (int)worldSize;
- (id <Entity>)worldspawn;
- (NSArray *)entities;

- (BOOL)postNotifications;
- (void)setPostNotifications:(BOOL)value;

- (Picker *)picker;
- (GLResources *)glResources;
@end

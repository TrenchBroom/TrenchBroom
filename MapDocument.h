//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Map.h"
#import "Math.h"

extern NSString* const FaceWillChange;
extern NSString* const FaceDidChange;
extern NSString* const FaceKey;

extern NSString* const BrushAdded;
extern NSString* const BrushWillBeRemoved;
extern NSString* const BrushWillChange;
extern NSString* const BrushDidChange;
extern NSString* const BrushKey;

extern NSString* const EntityAdded;
extern NSString* const EntityWillBeRemoved;
extern NSString* const EntityKey;

extern NSString* const PropertiesWillChange;
extern NSString* const PropertiesDidChange;

@class EntityDefinitionManager;
@class MutableEntity;
@class Picker;
@class GLResources;
@class Vector3i;
@class Vector3f;
@protocol Entity;
@protocol Brush;
@protocol Face;

@interface MapDocument : NSDocument <Map> {
    @private
    EntityDefinitionManager* entityDefinitionManager;
    NSMutableArray* entities;
    MutableEntity* worldspawn;
    int worldSize;
    BOOL postNotifications;
    Picker* picker;
    GLResources* glResources;
}

# pragma mark Texture wad management
- (void)insertObject:(NSString *)theWadPath inTextureWadsAtIndex:(NSUInteger)theIndex;
- (void)removeObjectFromTextureWadsAtIndex:(NSUInteger)theIndex;
- (NSArray *)textureWads;
- (void)updateTextureUsageCounts;

# pragma mark Map related functions

- (int)worldSize;
- (NSArray *)entities;
- (BOOL)postNotifications;
- (void)setPostNotifications:(BOOL)value;

- (id <Entity>)createEntityWithClassname:(NSString *)classname;
- (id <Entity>)createEntityWithProperties:(NSDictionary *)properties;
- (void)setEntity:(id <Entity>)entity propertyKey:(NSString *)key value:(NSString *)value;
- (void)setEntityDefinition:(id <Entity>)entity;
- (void)translateEntity:(id <Entity>)entity xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta;
- (void)translateEntity:(id <Entity>)entity direction:(const TVector3f *)dir delta:(int)delta;
- (void)deleteEntity:(id <Entity>)entity;

- (void)addBrushToEntity:(id <Entity>)theEntity brush:(id <Brush>)theBrush;
- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate;
- (void)translateBrush:(id <Brush>)brush xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta;
- (void)translateBrush:(id <Brush>)brush direction:(const TVector3f *)dir delta:(int)delta;
- (void)rotateZ90CW:(NSSet *)theBrushes entities:(NSSet *)theEntities;
- (void)rotateZ90CCW:(NSSet *)theBrushes entities:(NSSet *)theEntities;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter brushes:(NSSet *)theBrushes;
- (void)deleteBrush:(id <Brush>)brush;

- (void)setFace:(id <Face>)face xOffset:(int)xOffset;
- (void)setFace:(id <Face>)face yOffset:(int)yOffset;
- (void)translateFaceOffset:(id <Face>)face xDelta:(int)xDelta yDelta:(int)yDelta;
- (void)setFace:(id <Face>)face xScale:(float)xScale;
- (void)setFace:(id <Face>)face yScale:(float)yScale;
- (void)setFace:(id <Face>)face rotation:(float)angle;
- (void)setFace:(id <Face>)face texture:(NSString *)texture;
- (void)translateFace:(id <Face>)face xDelta:(int)xDelta yDelta:(int)yDelta zDelta:(int)zDelta;
- (void)dragFace:(id <Face>)face dist:(float)dist;

- (Picker *)picker;
- (GLResources *)glResources;
- (EntityDefinitionManager *)entityDefinitionManager;
@end

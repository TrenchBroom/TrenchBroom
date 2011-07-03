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

extern NSString* const FacesWillChange;
extern NSString* const FacesDidChange;
extern NSString* const FacesKey;

extern NSString* const BrushesAdded;
extern NSString* const BrushesWillBeRemoved;
extern NSString* const BrushesWillChange;
extern NSString* const BrushesDidChange;
extern NSString* const BrushesKey;

extern NSString* const EntitiesAdded;
extern NSString* const EntitiesWillBeRemoved;
extern NSString* const EntitiesKey;

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
- (void)setEntity:(id <Entity>)theEntity propertyKey:(NSString *)theKey value:(NSString *)theValue;
- (void)setEntityDefinition:(id <Entity>)entity;
- (void)translateEntities:(NSSet *)theEntities delta:(TVector3i *)theDelta;
- (void)translateEntities:(NSSet *)theEntities direction:(const TVector3f *)theDirection delta:(int)theDelta;
- (void)rotateEntitiesZ90CW:(NSSet *)theEntities center:(TVector3i *)theCenter;
- (void)rotateEntitiesZ90CCW:(NSSet *)theEntities center:(TVector3i *)theCenter;
- (void)deleteEntities:(NSSet *)theEntities;

- (void)addBrushesToEntity:(id <Entity>)theEntity brushes:(NSSet *)theBrushes;
- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate;
- (void)translateBrushes:(NSSet *)theBrushes delta:(TVector3i *)theDelta;
- (void)translateBrushes:(NSSet *)theBrushes direction:(const TVector3f *)theDirection delta:(int)theDelta;
- (void)rotateBrushesZ90CW:(NSSet *)theBrushes center:(TVector3i *)theCenter;
- (void)rotateBrushesZ90CCW:(NSSet *)theBrushes center:(TVector3i *)theCenter;
- (void)rotateBrushes:(NSSet *)theBrushes rotation:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter;
- (void)deleteBrushes:(NSSet *)theBrushes;

- (void)setFaces:(NSSet *)theFaces xOffset:(int)theXOffset;
- (void)setFaces:(NSSet *)theFaces yOffset:(int)theYOffset;
- (void)translateFaceOffsets:(NSSet *)theFaces xDelta:(int)theXDelta yDelta:(int)theYDelta;
- (void)setFaces:(NSSet *)theFaces xScale:(float)theXScale;
- (void)setFaces:(NSSet *)theFaces yScale:(float)theYScale;
- (void)scaleFaces:(NSSet *)theFaces xFactor:(float)theXFactor yFactor:(float)theYFactor;
- (void)setFaces:(NSSet *)theFaces rotation:(float)theAngle;
- (void)rotateFaces:(NSSet *)theFaces angle:(float)theAngle;
- (void)setFaces:(NSSet *)theFaces texture:(NSString *)theTexture;
- (void)translateFaces:(NSSet *)theFaces delta:(TVector3i *)theDelta;
- (void)dragFaces:(NSSet *)theFaces distance:(float)theDistance;

- (Picker *)picker;
- (GLResources *)glResources;
- (EntityDefinitionManager *)entityDefinitionManager;
@end

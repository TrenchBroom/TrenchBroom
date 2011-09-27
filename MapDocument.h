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
extern NSString* const BrushesWereRemoved;
extern NSString* const BrushesWillChange;
extern NSString* const BrushesDidChange;
extern NSString* const BrushesKey;

extern NSString* const EntitiesAdded;
extern NSString* const EntitiesWillBeRemoved;
extern NSString* const EntitiesWereRemoved;
extern NSString* const EntitiesKey;

extern NSString* const PropertiesWillChange;
extern NSString* const PropertiesDidChange;

extern NSString* const PointFileLoaded;
extern NSString* const PointFileUnloaded;

@class EntityDefinitionManager;
@class MutableEntity;
@class Picker;
@class GLResources;
@class Vector3i;
@class Vector3f;
@class SelectionManager;
@class GroupManager;
@protocol Entity;
@protocol Brush;
@protocol Face;

@interface MapDocument : NSDocument <Map> {
    @private
    SelectionManager* selectionManager;
    EntityDefinitionManager* entityDefinitionManager;
    GroupManager* groupManager;
    NSMutableArray* entities;
    MutableEntity* worldspawn;
    TBoundingBox worldBounds;
    BOOL postNotifications;
    Picker* picker;
    GLResources* glResources;
    
    TVector3f* leakPoints;
    int leakPointCount;
}

# pragma mark Point file support

- (void)loadPointFile:(NSData *)theData;
- (void)unloadPointFile;
- (TVector3f *)leakPoints;
- (int)leakPointCount;

# pragma mark Texture wad management

- (void)insertObject:(NSString *)theWadPath inTextureWadsAtIndex:(NSUInteger)theIndex;
- (void)removeObjectFromTextureWadsAtIndex:(NSUInteger)theIndex;
- (NSArray *)textureWads;
- (void)updateTextureUsageCounts;

# pragma mark Map related functions

- (NSArray *)entities;
- (BOOL)postNotifications;
- (void)setPostNotifications:(BOOL)value;

- (id <Entity>)createEntityWithClassname:(NSString *)classname;
- (id <Entity>)createEntityWithProperties:(NSDictionary *)properties;
- (void)duplicateEntities:(NSArray *)theEntities newEntities:(NSMutableArray *)theNewEntities newBrushes:(NSMutableArray *)theNewBrushes;
- (void)setEntity:(id <Entity>)theEntity propertyKey:(NSString *)theKey value:(NSString *)theValue;
- (void)setEntities:(NSArray *)theEntities propertyKey:(NSString *)theKey value:(NSString *)theValue;
- (void)setEntityDefinition:(id <Entity>)entity;
- (void)translateEntities:(NSArray *)theEntities delta:(TVector3i)theDelta;
- (void)rotateEntities90CW:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3i)theCenter;
- (void)rotateEntities90CCW:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3i)theCenter;
- (void)rotateEntities:(NSArray *)theEntities rotation:(TQuaternion)theRotation center:(TVector3f)theCenter;
- (void)flipEntities:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3i)theCenter;
- (void)deleteEntities:(NSArray *)theEntities;

- (void)addBrushesToEntity:(id <Entity>)theEntity brushes:(NSArray *)theBrushes;
- (void)moveBrushesToEntity:(id <Entity>)theEntity brushes:(NSArray *)theBrushes;
- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate;
- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity withBounds:(TBoundingBox *)theBounds texture:(NSString *)theTexture;
- (void)duplicateBrushes:(NSArray *)theBrushes newBrushes:(NSMutableArray *)theNewBrushes;
- (void)translateBrushes:(NSArray *)theBrushes delta:(TVector3i)theDelta lockTextures:(BOOL)lockTextures;
- (void)rotateBrushes90CW:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3i)theCenter lockTextures:(BOOL)lockTextures;
- (void)rotateBrushes90CCW:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3i)theCenter lockTextures:(BOOL)lockTextures;
- (void)rotateBrushes:(NSArray *)theBrushes rotation:(TQuaternion)theRotation center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures;
- (void)flipBrushes:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3i)theCenter lockTextures:(BOOL)lockTextures;
- (void)deleteBrushes:(NSArray *)theBrushes;

- (void)setFaces:(NSArray *)theFaces xOffset:(int)theXOffset;
- (void)setFaces:(NSArray *)theFaces yOffset:(int)theYOffset;
- (void)translateFaceOffsets:(NSArray *)theFaces xDelta:(int)theXDelta yDelta:(int)theYDelta;
- (void)setFaces:(NSArray *)theFaces xScale:(float)theXScale;
- (void)setFaces:(NSArray *)theFaces yScale:(float)theYScale;
- (void)scaleFaces:(NSArray *)theFaces xFactor:(float)theXFactor yFactor:(float)theYFactor;
- (void)setFaces:(NSArray *)theFaces rotation:(float)theAngle;
- (void)rotateFaces:(NSArray *)theFaces angle:(float)theAngle;
- (void)setFaces:(NSArray *)theFaces texture:(NSString *)theTexture;
- (void)dragFaces:(NSArray *)theFaces distance:(float)theDistance lockTextures:(BOOL)lockTextures;

- (void)clear;

- (Picker *)picker;
- (GLResources *)glResources;
- (EntityDefinitionManager *)entityDefinitionManager;
- (SelectionManager *)selectionManager;
- (GroupManager *)groupManager;
@end

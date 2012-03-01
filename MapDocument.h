/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>
#import "Map.h"
#import "Math.h"
#import "VertexData.h"

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

extern NSString* const DocumentCleared;
extern NSString* const DocumentLoaded;

@class EntityDefinitionManager;
@class MutableEntity;
@class Picker;
@class GLResources;
@class Vector3i;
@class Vector3f;
@class SelectionManager;
@class GroupManager;
@class Texture;
@class Autosaver;
@protocol Entity;
@protocol Brush;
@protocol Face;

@interface MapDocument : NSDocument <Map> {
    @private
    SelectionManager* selectionManager;
    EntityDefinitionManager* entityDefinitionManager;
    GroupManager* groupManager;
    Autosaver* autosaver;
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
- (void)updateFaceTextures;

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
- (void)translateEntities:(NSArray *)theEntities delta:(TVector3f)theDelta;
- (void)rotateEntities90CW:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3f)theCenter;
- (void)rotateEntities90CCW:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3f)theCenter;
- (void)rotateEntities:(NSArray *)theEntities rotation:(TQuaternion)theRotation center:(TVector3f)theCenter;
- (void)flipEntities:(NSArray *)theEntities axis:(EAxis)theAxis center:(TVector3f)theCenter;
- (void)deleteEntities:(NSArray *)theEntities;

- (void)addBrushesToEntity:(id <Entity>)theEntity brushes:(NSArray *)theBrushes;
- (void)moveBrushesToEntity:(id <Entity>)theEntity brushes:(NSArray *)theBrushes;
- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity fromTemplate:(id <Brush>)theTemplate;
- (id <Brush>)createBrushInEntity:(id <Entity>)theEntity withBounds:(TBoundingBox *)theBounds texture:(Texture *)theTexture;
- (void)duplicateBrushes:(NSArray *)theBrushes newBrushes:(NSMutableArray *)theNewBrushes;
- (void)translateBrushes:(NSArray *)theBrushes delta:(TVector3f)theDelta lockTextures:(BOOL)lockTextures;
- (void)rotateBrushes90CW:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures;
- (void)rotateBrushes90CCW:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures;
- (void)rotateBrushes:(NSArray *)theBrushes rotation:(TQuaternion)theRotation center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures;
- (void)flipBrushes:(NSArray *)theBrushes axis:(EAxis)theAxis center:(TVector3f)theCenter lockTextures:(BOOL)lockTextures;
- (void)snapBrushes:(NSArray *)theBrushes;
- (void)deleteBrushes:(NSArray *)theBrushes;

- (void)setFaces:(NSArray *)theFaces xOffset:(int)theXOffset;
- (void)setFaces:(NSArray *)theFaces yOffset:(int)theYOffset;
- (void)translateFaceOffsets:(NSArray *)theFaces delta:(float)theDelta dir:(TVector3f)theDir;
- (void)setFaces:(NSArray *)theFaces xScale:(float)theXScale;
- (void)setFaces:(NSArray *)theFaces yScale:(float)theYScale;
- (void)scaleFaces:(NSArray *)theFaces xFactor:(float)theXFactor yFactor:(float)theYFactor;
- (void)setFaces:(NSArray *)theFaces rotation:(float)theAngle;
- (void)rotateFaces:(NSArray *)theFaces angle:(float)theAngle;
- (void)setFaces:(NSArray *)theFaces texture:(Texture *)theTexture;
- (BOOL)dragFaces:(NSArray *)theFaces distance:(float)theDistance lockTextures:(BOOL)lockTextures;

- (TDragResult)dragVertex:(int)theVertexIndex brush:(id <Brush>)theBrush delta:(const TVector3f *)theDelta;
- (TDragResult)dragEdge:(int)theEdgeIndex brush:(id <Brush>)theBrush delta:(const TVector3f *)theDelta;
- (TDragResult)dragFace:(int)theFaceIndex brush:(id <Brush>)theBrush delta:(const TVector3f *)theDelta;

- (void)clear;

- (Picker *)picker;
- (GLResources *)glResources;
- (EntityDefinitionManager *)entityDefinitionManager;
- (SelectionManager *)selectionManager;
- (GroupManager *)groupManager;
@end

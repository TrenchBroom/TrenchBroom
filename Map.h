//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

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

@interface Map : NSObject {
    @private
    NSMutableArray* entities;
    Entity* worldspawn;
    int worldSize;
    BOOL postNotifications;
    NSUndoManager* undoManager;
}

- (Entity *)worldspawn;

- (Entity *)createEntity;
- (Entity *)createEntityWithProperty:(NSString *)key value:(NSString *)value;
- (void)removeEntity:(Entity *)entity;

- (NSArray* )entities;
- (int)worldSize;

- (NSSet *)textureNames;

- (BOOL)postNotifications;
- (void)setPostNotifications:(BOOL)value;

- (NSUndoManager *)undoManager;
- (void)setUndoManager:(NSUndoManager *)theUndoManager;

- (void)faceFlagsChanged:(Face *)face;
- (void)faceTextureChanged:(Face *)face oldTexture:(NSString *)oldTexture newTexture:(NSString *)newTexture;
- (void)faceGeometryChanged:(Face *)face;
- (void)faceAdded:(Face *)face;
- (void)faceRemoved:(Face *)face;

- (void)brushAdded:(Brush *)brush;
- (void)brushRemoved:(Brush *)brush;

- (void)propertyAdded:(Entity *)entity key:(NSString *)key value:(NSString *)value;
- (void)propertyRemoved:(Entity *)entity key:(NSString *)key value:(NSString *)value;
- (void)propertyChanged:(Entity *)entity key:(NSString *)key oldValue:(NSString *)oldValue newValue:(NSString *)newValue;

@end

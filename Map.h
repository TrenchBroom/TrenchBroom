//
//  Map.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"

extern NSString* const MapEntityAdded;
extern NSString* const MapEntityRemoved;
extern NSString* const EntityKey;

@class Entity;

@interface Map : Observable {
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

- (BOOL)postNotifications;

/*!
    @function
    @abstract   Returns the undo manager for this map.
    @result     The undo manager for this map or nil if there is no undo manager.
*/
- (NSUndoManager *)undoManager;

/*!
    @function
    @abstract   Sets the undo manager for this map.
*/
- (void)setUndoManager:(NSUndoManager *)theUndoManager;

- (void)setPostNotifications:(BOOL)value;
@end

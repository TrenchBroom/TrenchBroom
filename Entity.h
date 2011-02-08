//
//  Entitiy.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"

extern NSString* const EntityBrushAdded;
extern NSString* const EntityBrushRemoved;
extern NSString* const BrushKey;
extern NSString* const EntityPropertyAdded;
extern NSString* const EntityPropertyRemoved;
extern NSString* const EntityPropertyChanged;
extern NSString* const PropertyNameKey;
extern NSString* const PropertyValueKey;
extern NSString* const PropertyOldValueKey;

@class Vector3i;
@class Map;
@class Brush;

@interface Entity : Observable {
    @private
    Map* map;
    NSNumber* entityId;
	NSMutableArray* brushes;
    NSMutableDictionary* brushIndices;
	NSMutableDictionary* properties;
}

- (id)initInMap:(Map *)theMap;
- (id)initInMap:(Map *)theMap property:(NSString *)key value:(NSString *)value;

- (Brush *)createBrush;
- (void)addBrush:(Brush *)brush;
- (void)removeBrush:(Brush *)brush;

- (Map *)map;
- (NSNumber *)entityId;

- (NSArray *)brushes;

- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;
- (NSString *)propertyForKey:(NSString *)key;
- (NSString *)classname;

- (NSDictionary *)properties;

- (BOOL)isWorldspawn;

- (BOOL)postNotifications;

/*!
    @function
    @abstract   Returns the undo manager for this entity.
    @discussion This method simply returns the undo manager of the map to which this entity belongs.
    @result     The undo manager for this entity or nil if there is no undo manager.
*/
- (NSUndoManager *)undoManager;
@end

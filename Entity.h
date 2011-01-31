//
//  Entitiy.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const EntityBrushAdded;
extern NSString* const EntityBrushRemoved;
extern NSString* const EntityBrushChanged;
extern NSString* const EntityBrushKey;
extern NSString* const EntityPropertyAdded;
extern NSString* const EntityPropertyRemoved;
extern NSString* const EntityPropertyChanged;
extern NSString* const EntityPropertyNameKey;
extern NSString* const EntityPropertyValueKey;
extern NSString* const EntityPropertyOldValueKey;

@class Vector3i;
@class Map;
@class Brush;

@interface Entity : NSObject {
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
@end

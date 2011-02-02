//
//  Entitiy.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Entity.h"
#import "Map.h"
#import "IdGenerator.h"
#import "Vector3i.h"
#import "Brush.h"

NSString* const EntityBrushAdded            = @"EntityBrushAdded";
NSString* const EntityBrushRemoved          = @"EntityBrushRemoved";
NSString* const EntityBrushKey              = @"EntityBrushKey";
NSString* const EntityPropertyAdded         = @"EntityPropertyAdded";
NSString* const EntityPropertyRemoved       = @"EntityPropertyRemoved";
NSString* const EntityPropertyChanged       = @"EntityPropertyChanged";
NSString* const EntityPropertyKeyKey        = @"EntityPropertyKeyKey";
NSString* const EntityPropertyValueKey      = @"EntityPropertyValueKey";
NSString* const EntityPropertyOldValueKey   = @"EntityPropertyOldValueKey";


@implementation Entity

- (id)init {
    if (self = [super init]) {
        entityId = [[[IdGenerator sharedGenerator] getId] retain];
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableArray alloc] init];
        brushIndices = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}
- (id)initInMap:(Map *)theMap {
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];
    
    if (self = [self init]) {
        map = theMap; // do not retain
    }
    
    return self;
}

- (id)initInMap:(Map *)theMap property:(NSString *)key value:(NSString *)value {
	if (self = [self initInMap:theMap]) {
		[self setProperty:key value:value];
	}
	
	return self;
}

- (Brush *)createBrush {
    Brush* brush = [[Brush alloc] initInEntity:self];
    [brushes addObject:brush];
    [brushIndices setObject:[NSNumber numberWithInt:[brushes count] - 1] forKey:[brush brushId]];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityBrushAdded object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:EntityBrushKey]];
    }
    
    return [brush autorelease];
}

- (void)removeBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
 
    NSNumber* index = [brushIndices objectForKey:[brush brushId]];
    if (index == nil)
        [NSException raise:NSInvalidArgumentException format:@"Entity %@ does not contain brush %@", self, brush];
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:brush forKey:EntityBrushKey];
    [brushes removeObjectAtIndex:[index intValue]];
    [brushIndices removeObjectForKey:[brush brushId]];
    
    if ([self postNotifications]) {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityBrushRemoved object:self userInfo:userInfo];
    }
}

- (Map *)map {
    return map;
}

- (NSNumber *)entityId {
    return entityId;
}

- (NSArray *)brushes {
    return brushes;
}

- (void)setProperty:(NSString *)key value:(NSString *)value {
    NSString* oldValue = [self propertyForKey:key];
    BOOL exists = oldValue != nil;
    
    if (exists && [oldValue isEqualToString:value])
        return;
    
    [properties setObject:value forKey:key];

    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:key forKey:EntityPropertyKeyKey];
        [userInfo setObject:value forKey:EntityPropertyValueKey];
    
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        if (exists) {
            [userInfo setObject:oldValue forKey:EntityPropertyOldValueKey];
            [center postNotificationName:EntityPropertyChanged object:self userInfo:userInfo];
        } else {
            [center postNotificationName:EntityPropertyAdded object:self userInfo:userInfo];
        }
    }
}

- (void)removeProperty:(NSString *)key {
    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    [properties removeObjectForKey:key];

    if ([self postNotifications]) {
        NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:key forKey:EntityPropertyKeyKey];
        [userInfo setObject:oldValue forKey:EntityPropertyValueKey];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:EntityPropertyAdded object:self userInfo:userInfo];
    }
}

- (NSString *)propertyForKey:(NSString *)key {
    return (NSString *)[properties objectForKey:key];
}

- (NSString *)classname {
    return [self propertyForKey:@"classname"];
}

- (NSDictionary *)properties {
    return properties;
}

- (BOOL)isWorldspawn {
    return [[self classname] isEqualToString:@"worldspawn"];
}

- (BOOL)postNotifications {
    return [map postNotifications];
}

- (void) dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [entityId release];
	[properties release];
	[brushes release];
    [brushIndices release];
	[super dealloc];
}

@end

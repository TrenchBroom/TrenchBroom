//
//  Entitiy.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Entity.h"
#import "IdGenerator.h"
#import "Vector3i.h"
#import "Brush.h"

NSString* const EntityBrushAdded = @"BrushAdded";
NSString* const EntityBrushRemoved = @"BrushRemoved";

NSString* const EntityBrush = @"Brush";

NSString* const EntityPropertyAdded = @"PropertyAdded";
NSString* const EntityPropertyRemoved = @"PropertyRemoved";
NSString* const EntityPropertyChanged = @"PropertyChanged";

NSString* const EntityPropertyKey = @"PropertyKey";
NSString* const EntityPropertyNewValue = @"PropertyNewValue";
NSString* const EntityPropertyOldValue = @"PropertyOldValue";


@implementation Entity

- (id)init {
    if (self = [super init]) {
        entityId = [[IdGenerator sharedGenerator] getId];
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (id)initWithProperty:(NSString *)key value:(NSString *)value {
	if (self = [self init]) {
		[self setProperty:key value:value];
	}
	
	return self;
}

- (Brush *)createCuboidAt:(Vector3i *)position dimensions:(Vector3i *)dimensions texture:(NSString *)texture {
    Brush* brush = [[Brush alloc] initCuboidAt:position dimensions:dimensions texture:texture];
    [self addBrush:brush];
    
    return [brush autorelease];
}

- (Brush *)createBrush {
    Brush* brush = [[Brush alloc] init];
    [self addBrush:brush];
    
    return [brush autorelease];
}

- (void)addBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    [brushes addObject:brush];
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:1];
    [info setObject:brush forKey:EntityBrush];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:EntityBrushAdded object:self userInfo:info];
}

- (void)removeBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    if (![brushes containsObject:brush])
        return;
    
    [brushes removeObject:brush];

    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:1];
    [info setObject:brush forKey:EntityBrush];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:EntityBrushRemoved object:self userInfo:info];
}

- (NSNumber *)getId {
    return entityId;
}

- (NSSet *)brushes {
    return brushes;
}

- (void)setProperty:(NSString *)key value:(NSString *)value {
    NSString *oldValue = [self propertyForKey:key];
    BOOL exists = oldValue != nil;
    
    if (exists && [oldValue isEqualToString:value])
        return;
    
    [properties setObject:value forKey:key];
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:3];
    [info setObject:key forKey:EntityPropertyKey];
    [info setObject:value forKey:EntityPropertyNewValue];
    
    if (exists) {
        [info setObject:oldValue forKey:EntityPropertyOldValue];
        [[NSNotificationCenter defaultCenter] postNotificationName:EntityPropertyChanged object:self userInfo:info];
    } else {
        [[NSNotificationCenter defaultCenter] postNotificationName:EntityPropertyAdded object:self userInfo:info];
    }
}

- (void)removeProperty:(NSString *)key {
    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    [properties removeObjectForKey:key];
    
    NSMutableDictionary* info = [NSMutableDictionary dictionaryWithCapacity:2];
    [info setObject:key forKey:EntityPropertyKey];
    [info setObject:oldValue forKey:EntityPropertyOldValue];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:EntityPropertyRemoved object:self userInfo:info];
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

- (void) dealloc {
	[properties release];
	[brushes release];
	[super dealloc];
}

@end

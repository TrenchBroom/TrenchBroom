//
//  Entitiy.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Entity.h"
#import "Map.h"
#import "Brush.h"
#import "Face.h"
#import "IdGenerator.h"
#import "Vector3i.h"

NSString* const EntityBrushAdded            = @"EntityBrushAdded";
NSString* const EntityBrushRemoved          = @"EntityBrushRemoved";
NSString* const BrushKey                    = @"EntityBrushKey";
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
    [self addBrush:brush];
    
    return [brush autorelease];
}

- (void)addBrush:(Brush *)brush {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] removeBrush:brush];
    
    [brushes addObject:brush];
    [brushIndices setObject:[NSNumber numberWithInt:[brushes count] - 1] forKey:[brush brushId]];

    [self addForward:BrushFaceAdded from:brush];
    [self addForward:BrushFaceAdded from:brush];
    [self addForward:FaceGeometryChanged from:brush];
    [self addForward:FaceFlagsChanged from:brush];
    
    [self notifyObservers:EntityBrushAdded infoObject:brush infoKey:BrushKey];
}

- (void)removeBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
 
    NSNumber* index = [brushIndices objectForKey:[brush brushId]];
    if (index == nil)
        [NSException raise:NSInvalidArgumentException format:@"Entity %@ does not contain brush %@", self, brush];
 
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] addBrush:brush];
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:brush forKey:BrushKey];
    [brush removeObserver:self];
    [brushes removeObjectAtIndex:[index intValue]];
    [brushIndices removeObjectForKey:[brush brushId]];
    
    [self notifyObservers:EntityBrushRemoved userInfo:userInfo];
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
    
    NSUndoManager* undoManager = [self undoManager];
    if (exists)
        [[undoManager prepareWithInvocationTarget:self] setProperty:key value:oldValue];
    else 
        [[undoManager prepareWithInvocationTarget:self] removeProperty:key];
    
    [properties setObject:value forKey:key];

    NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
    [userInfo setObject:key forKey:EntityPropertyKeyKey];
    [userInfo setObject:value forKey:EntityPropertyValueKey];
    
    if (exists) {
        [userInfo setObject:oldValue forKey:EntityPropertyOldValueKey];
        [self notifyObservers:EntityPropertyChanged userInfo:userInfo];
    } else {
        [self notifyObservers:EntityPropertyAdded userInfo:userInfo];
    }
}

- (void)removeProperty:(NSString *)key {
    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setProperty:key value:oldValue];
    
    NSMutableDictionary* userInfo = [NSMutableDictionary dictionary];
    [userInfo setObject:key forKey:EntityPropertyKeyKey];
    [userInfo setObject:oldValue forKey:EntityPropertyValueKey];

    [properties removeObjectForKey:key];

    [self notifyObservers:EntityPropertyRemoved userInfo:userInfo];
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

- (NSUndoManager *)undoManager {
    return [map undoManager];
}

- (void) dealloc {
    NSEnumerator* brushEn = [brushes objectEnumerator];
    Brush* brush;
    while ((brush = [brushEn nextObject]))
        [brush removeObserver:self];
    
    [entityId release];
	[properties release];
	[brushes release];
    [brushIndices release];
	[super dealloc];
}

@end

//
//  Entitiy.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Entity.h"
#import "MapDocument.h"
#import "Brush.h"
#import "Face.h"
#import "IdGenerator.h"
#import "Vector3i.h"

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
- (id)initInMap:(MapDocument *)theMap {
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];
    
    if (self = [self init]) {
        map = theMap; // do not retain
    }
    
    return self;
}

- (id)initInMap:(MapDocument *)theMap property:(NSString *)key value:(NSString *)value {
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

    [map brushAdded:brush];
}

- (void)removeBrush:(Brush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
 
    NSNumber* index = [brushIndices objectForKey:[brush brushId]];
    if (index == nil)
        [NSException raise:NSInvalidArgumentException format:@"Entity %@ does not contain brush %@", self, brush];
 
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] addBrush:brush];
    
    [brushes removeObjectAtIndex:[index intValue]];
    [brushIndices removeObjectForKey:[brush brushId]];
    
    [map brushRemoved:brush];
}

- (MapDocument *)map {
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
    
    if (exists)
        [map propertyChanged:self key:key oldValue:oldValue newValue:value];
    else
        [map propertyAdded:self key:key value:value];
}

- (void)removeProperty:(NSString *)key {
    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] setProperty:key value:oldValue];
    
    [properties removeObjectForKey:key];
    [map propertyRemoved:self key:key value:oldValue];
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

- (NSUndoManager *)undoManager {
    return [map undoManager];
}

- (void)faceFlagsChanged:(Face *)face {
    [map faceFlagsChanged:face];
}

- (void)faceTextureChanged:(Face *)face oldTexture:(NSString *)oldTexture newTexture:(NSString *)newTexture {
    [map faceTextureChanged:face oldTexture:oldTexture newTexture:newTexture];
}

- (void)faceGeometryChanged:(Face *)face {
    [map faceGeometryChanged:face];
}

- (void)faceAdded:(Face *)face {
    [map faceAdded:face];
}

- (void)faceRemoved:(Face *)face {
    [map faceRemoved:face];
}

- (void) dealloc {
    [entityId release];
	[properties release];
	[brushes release];
    [brushIndices release];
	[super dealloc];
}

@end

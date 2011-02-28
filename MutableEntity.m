//
//  Entitiy.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MutableEntity.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "IdGenerator.h"
#import "Vector3i.h"
#import "Vector3f.h"
#import "BoundingBox.h"

@implementation MutableEntity

- (id)init {
    if (self = [super init]) {
        entityId = [[[IdGenerator sharedGenerator] getId] retain];
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableArray alloc] init];
        brushIndices = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)addBrush:(MutableBrush *)brush {
    [brushes addObject:brush];
    [brushIndices setObject:[NSNumber numberWithInt:[brushes count] - 1] forKey:[brush brushId]];
    [brush setEntity:self];
}

- (void)removeBrush:(MutableBrush *)brush {
    if (brush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
 
    NSNumber* index = [brushIndices objectForKey:[brush brushId]];
    if (index == nil)
        [NSException raise:NSInvalidArgumentException format:@"Entity %@ does not contain brush %@", self, brush];
 
    [brush setEntity:nil];
    [brushes removeObjectAtIndex:[index intValue]];
    [brushIndices removeObjectForKey:[brush brushId]];
}

- (NSNumber *)entityId {
    return entityId;
}

- (id <Map>)map {
    return map;
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
}

- (void)removeProperty:(NSString *)key {
    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    [properties removeObjectForKey:key];
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

- (BoundingBox *)bounds {
    if (bounds == nil && [brushes count] > 0) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        MutableBrush* brush = [brushEn nextObject];
        
        bounds = [[BoundingBox alloc] initWithMin:[[brush bounds] min] max:[[brush bounds] max]];
        while ((brush = [brushEn nextObject]))
            [bounds merge:[brush bounds]];
    }
    
    return bounds;
}

- (Vector3f *)center {
    if (center == nil && [brushes count] > 0) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        MutableBrush* brush = [brushEn nextObject];
        
        center = [[Vector3f alloc] initWithFloatVector:[brush center]];
        while ((brush = [brushEn nextObject]))
            [center add:[brush center]];
        
        [center scale:1.0f / [brushes count]];
    }
    
    return center;
}

- (void)setMap:(id <Map>)theMap {
    map = theMap;
}

- (void) dealloc {
    [entityId release];
	[properties release];
	[brushes release];
    [brushIndices release];
    [center release];
    [bounds release];
	[super dealloc];
}

@end

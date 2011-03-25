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
    }
    
    return self;
}

- (id)initWithProperties:(NSDictionary *)theProperties {
    if (self = [self init]) {
        [properties addEntriesFromDictionary:theProperties];
    }
    
    return self;
}

- (void)addBrush:(MutableBrush *)brush {
    [brushes addObject:brush];
    [brush setEntity:self];
}

- (void)removeBrush:(MutableBrush *)brush {
    [brush setEntity:nil];
    [brushes removeObject:brush];
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
        
        bounds = [[BoundingBox alloc] initWithBounds:[brush bounds]];
        while ((brush = [brushEn nextObject]))
            [bounds mergeBounds:[brush bounds]];
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
    [center release];
    [bounds release];
	[super dealloc];
}

@end

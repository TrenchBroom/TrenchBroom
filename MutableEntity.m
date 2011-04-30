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
#import "VBOMemBlock.h"

@interface MutableEntity (private)

- (void)validate;

@end

@implementation MutableEntity (private)

- (void)validate {
    if ([brushes count] > 0) {
        NSEnumerator* brushEn = [brushes objectEnumerator];
        MutableBrush* brush = [brushEn nextObject];
        
        bounds = *[brush bounds];
        center = *[brush center];
        while ((brush = [brushEn nextObject])) {
            mergeBoundsWithBounds(&bounds, [brush bounds], &bounds);
            addV3f(&center, [brush center], &center);
        }
        
        scaleV3f(&center, 1.0f / [brushes count], &center);
    } else {
        center = NullVector;
        bounds.min = NullVector;
        bounds.max = NullVector;
    }
}

@end

@implementation MutableEntity

- (id)init {
    if (self = [super init]) {
        entityId = [[[IdGenerator sharedGenerator] getId] retain];
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableArray alloc] init];
        memBlocks = [[NSMutableDictionary alloc] init];
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
    valid = NO;
}

- (void)removeBrush:(MutableBrush *)brush {
    [brush setEntity:nil];
    [brushes removeObject:brush];
    valid = NO;
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

- (void)setMap:(id <Map>)theMap {
    map = theMap;
}

- (void) dealloc {
    [entityId release];
	[properties release];
	[brushes release];
    [memBlocks release];
	[super dealloc];
}

#pragma mark -
#pragma mark @implementation Entity

- (NSNumber *)entityId {
    return entityId;
}

- (id <Map>)map {
    return map;
}

- (NSArray *)brushes {
    return brushes;
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

- (TBoundingBox *)bounds {
    if (!valid)
        [self validate];
    
    return &bounds;
}

- (TVector3f *)center {
    if (!valid)
        [self validate];

    return &center;
}

- (void)setMemBlock:(VBOMemBlock *)theBlock forKey:(id <NSCopying>)theKey {
    VBOMemBlock* oldBlock = [memBlocks objectForKey:theKey];
    if (oldBlock != nil) {
        [oldBlock free];
        [memBlocks removeObjectForKey:theKey];
    }
        
    if (theBlock != nil)
        [memBlocks setObject:theBlock forKey:theKey];
}

- (VBOMemBlock *)memBlockForKey:(id <NSCopying>)theKey {
    return [memBlocks objectForKey:theKey];
}

@end

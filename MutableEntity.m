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
#import "PickingHitList.h"
#import "PickingHit.h"
#import "EntityDefinition.h"
#import "EntityDefinitionManager.h"

@interface MutableEntity (private)

- (void)validate;

@end

@implementation MutableEntity (private)

- (void)validate {
    if (entityDefinition == nil || [entityDefinition type] == EDT_BRUSH) {
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
            bounds.min = NullVector;
            bounds.max = NullVector;
            center = NullVector;
        }
    } else if ([entityDefinition type] == EDT_POINT) {
        bounds = *[entityDefinition bounds];
        
        TVector3f of;
        setV3f(&of, &origin);
        
        addV3f(&bounds.min, &of, &bounds.min);
        addV3f(&bounds.max, &of, &bounds.max);
    } else {
        bounds.min = NullVector;
        bounds.max = NullVector;
        center = NullVector;
    }
    
    valid = YES;
}

@end

@implementation MutableEntity

- (id)init {
    if (self = [super init]) {
        entityId = [[[IdGenerator sharedGenerator] getId] retain];
		properties = [[NSMutableDictionary alloc] init];
		brushes = [[NSMutableArray alloc] init];
        filePosition = -1;
    }
    
    return self;
}

- (id)initWithProperties:(NSDictionary *)theProperties {
    if (self = [self init]) {
        NSEnumerator* keyEn = [[theProperties allKeys] objectEnumerator];
        NSString* key;
        while ((key = [keyEn nextObject])) {
            NSString* value = [theProperties objectForKey:key];
            [self setProperty:key value:value];
        }
    }
    
    return self;
}

- (void)addBrush:(MutableBrush *)brush {
    NSAssert(brush != nil, @"brush must not be nil");
    
    if (entityDefinition != nil && [entityDefinition type] != EDT_BRUSH)
            [NSException raise:NSInternalInconsistencyException format:@"Cannot add brush to point or base entity"];
    
    
    [brushes addObject:brush];
    [brush setEntity:self];
    valid = NO;
}

- (void)removeBrush:(MutableBrush *)brush {
    NSAssert(brush != nil, @"brush must not be nil");

    if (entityDefinition != nil && [entityDefinition type] != EDT_BRUSH)
            [NSException raise:NSInternalInconsistencyException format:@"Cannot remove brush from point or base entity"];
    
    [brush setEntity:nil];
    [brushes removeObject:brush];
    valid = NO;
}

- (void)brushChanged:(MutableBrush *)brush {
    valid = NO;
}

- (void)setProperty:(NSString *)key value:(NSString *)value {
    NSAssert(key != nil, @"property key must not be nil");
    
    if ([key isEqualToString:ClassnameKey] && [self classname] != nil) {
        NSLog(@"Cannot overwrite classname property");
        return;
    } else if ([key isEqualToString:OriginKey]) {
        if (!parseV3i(value, NSMakeRange(0, [value length]), &origin)) {
            NSLog(@"Invalid origin value: '&@'", value);
            return;
        }
        valid = NO;
    } else if ([key isEqualToString:AngleKey]) {
        [angle release];
        if (value != nil)
            angle = [[NSNumber alloc] initWithInt:[value intValue]];
        else
            angle = nil;
    }
    
    NSString* oldValue = [self propertyForKey:key];
    BOOL exists = oldValue != nil;
    
    if (exists && [oldValue isEqualToString:value])
        return;
    
    [properties setObject:value forKey:key];
    valid = NO;
}

- (void)removeProperty:(NSString *)key {
    NSAssert(key != nil, @"property key must not be nil");
    
    if ([key isEqualToString:ClassnameKey]) {
        NSLog(@"Cannot delete classname property");
        return;
    } else if ([key isEqualToString:OriginKey]) {
        NSLog(@"Cannot delete origin property");
        return;
    } else if ([key isEqualToString:AngleKey]) {
        [angle release];
        angle = nil;
    }

    NSString *oldValue = [self propertyForKey:key];
    if (oldValue == nil)
        return;
    
    [properties removeObjectForKey:key];
    valid = NO;
}

- (void)setEntityDefinition:(EntityDefinition *)theEntityDefinition {
    NSAssert(theEntityDefinition != nil, @"entity definition must not be nil");
    NSAssert(entityDefinition == nil, @"can't change entity definition");
    
    entityDefinition = [theEntityDefinition retain];
}

- (void)setMap:(id <Map>)theMap {
    map = theMap;
}

- (int)filePosition {
    return filePosition;
}

- (void)setFilePosition:(int)theFilePosition {
    filePosition = theFilePosition;
}

- (void) dealloc {
    [entityId release];
	[properties release];
	[brushes release];
    [entityDefinition release];
    [angle release];
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

- (NSDictionary *)properties {
    return properties;
}

- (NSString *)classname {
    return [self propertyForKey:ClassnameKey];
}

- (EntityDefinition *)entityDefinition {
    return entityDefinition;
}

- (BOOL)isWorldspawn {
    return [[self classname] isEqualToString:WorldspawnClassname];
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

- (TVector3i *)origin {
    if (entityDefinition == nil || [entityDefinition type] != EDT_POINT)
        [NSException raise:NSInternalInconsistencyException format:@"Entity is not a point entity (ID %@)", entityId];
    
    return &origin;
}

- (NSNumber *)angle {
    return angle;
}

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList {
    if ([self isWorldspawn])
        return;
    
    float dist = intersectBoundsWithRay([self bounds], theRay, NULL);
    if (isnan(dist))
        return;
    
    TVector3f hitPoint;
    rayPointAtDistance(theRay, dist, &hitPoint);

    PickingHit* pickingHit = [[PickingHit alloc] initWithObject:self type:HT_ENTITY hitPoint:&hitPoint distance:dist];
    [theHitList addHit:pickingHit];
    [pickingHit release];
}

@end

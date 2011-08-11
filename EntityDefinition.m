//
//  EntityDefinition.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinition.h"
#import "ModelProperty.h"
#import "Entity.h"
#import "SpawnFlag.h"

@implementation EntityDefinition

- (id)initBaseDefinitionWithName:(NSString *)theName flags:(NSDictionary *)theFlags properties:(NSArray *)theProperties {
    NSAssert(theName != nil, @"name must not be nil");
    
    if ((self = [self init])) {
        name = [theName retain];
        flags = [theFlags retain];
        properties = [theProperties retain];
        type = EDT_BASE;
        center = NullVector;
        bounds.min = NullVector;
        bounds.max = NullVector;
        maxBounds.min = NullVector;
        maxBounds.max = NullVector;
    }
    
    return self;
}

- (id)initPointDefinitionWithName:(NSString *)theName color:(float *)theColor bounds:(TBoundingBox *)theBounds flags:(NSDictionary *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theColor != NULL, @"color must not be null");
    NSAssert(theBounds != nil, @"bounds must not be nil");
    
    if ((self = [self init])) {
        name = [theName retain];
        memcpy(color, theColor, 3 * sizeof(float));
        bounds = *theBounds;
        flags = [theFlags retain];
        properties = [theProperties retain];
        description = [theDescription retain];
        type = EDT_POINT;

        TVector3f diff;
        centerOfBounds(&bounds, &center);
        
        subV3f(&bounds.max, &center, &diff);
        float dist = lengthV3f(&diff);
        diff.x = dist;
        diff.y = dist;
        diff.z = dist;
        subV3f(&center, &diff, &maxBounds.min);
        addV3f(&center, &diff, &maxBounds.max);
    }
    
    return self;
}

- (id)initBrushDefinitionWithName:(NSString *)theName color:(float *)theColor flags:(NSDictionary *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theColor != NULL, @"color must not be null");
    
    if ((self = [self init])) {
        name = [theName retain];
        memcpy(color, theColor, 3 * sizeof(float));
        flags = [theFlags retain];
        properties = [theProperties retain];
        description = [theDescription retain];
        type = EDT_BRUSH;
        center = NullVector;
        bounds.min = NullVector;
        bounds.max = NullVector;
        maxBounds.min = NullVector;
        maxBounds.max = NullVector;
    }
    
    return self;
}

- (EEntityDefinitionType)type {
    return type;
}

- (NSString *)name {
    return name;
}

- (float *)color {
    return color;
}

- (const TVector3f *)center {
    return &center;
}

- (const TBoundingBox *)maxBounds {
    return &maxBounds;
}

- (const TBoundingBox *)bounds {
    return &bounds;
}

- (SpawnFlag *)flagForName:(NSString *)theName {
    return [flags objectForKey:theName];
}

- (NSArray *)flagsForMask:(int)theMask {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    
    NSEnumerator* flagEn = [flags objectEnumerator];
    SpawnFlag* flag;
    while ((flag = [flagEn nextObject])) {
        if ((theMask & [flag flag]) != 0)
            [result addObject:flag];
    }
    
    [result sortUsingSelector:@selector(compareByFlag:)];
    return [result autorelease];
}

- (NSArray *)allFlags {
    NSMutableArray* result = [[NSMutableArray alloc] initWithArray:[flags allValues]];
    [result sortUsingSelector:@selector(compareByFlag:)];
    return [result autorelease];
}

- (BOOL)isFlag:(NSString *)theFlagName setOnEntity:(id <Entity>)theEntity {
    SpawnFlag* flag = [self flagForName:theFlagName];
    if (flag == nil)
        return NO;
    
    NSString* entityFlagsStr = [theEntity propertyForKey:SpawnFlagsKey];
    if (entityFlagsStr == nil)
        return NO;
    
    int entityFlags = [entityFlagsStr intValue];
    return ([flag flag] & entityFlags) != 0;
}

- (NSArray *)properties {
    return properties;
}

- (ModelProperty *)modelPropertyForEntity:(id <Entity>)theEntity {
    NSAssert(theEntity != nil, @"entity must not be nil");
    
    ModelProperty* defaultProperty = nil;
    ModelProperty* specificProperty = nil;
    
    NSEnumerator* propertyEn = [properties objectEnumerator];
    id <EntityDefinitionProperty> property;
    while ((property = [propertyEn nextObject]) && (specificProperty == nil)) {
        if ([property isKindOfClass:[ModelProperty class]]) {
            ModelProperty* modelProperty = (ModelProperty *)property;
            NSString* flagName = [modelProperty flagName];
            if (flagName == nil)
                defaultProperty = modelProperty;
            else if ([self isFlag:flagName setOnEntity:theEntity])
                specificProperty = modelProperty;
        }
    }
    
    return specificProperty != nil ? specificProperty : defaultProperty;
}

- (ModelProperty *)defaultModelProperty {
    NSEnumerator* propertyEn = [properties objectEnumerator];
    id <EntityDefinitionProperty> property;
    while ((property = [propertyEn nextObject])) {
        if ([property isKindOfClass:[ModelProperty class]]) {
            ModelProperty* modelProperty = (ModelProperty *)property;
            NSString* flagName = [modelProperty flagName];
            if (flagName == nil)
                return modelProperty;
        }
    }
    
    return nil;
}

- (void)incUsageCount {
    usageCount++;
}

- (void)decUsageCount {
    usageCount--;
}
    
- (int)usageCount {
    return usageCount;
}

- (NSString *)description {
    return description;
}

- (NSComparisonResult)compareByName:(EntityDefinition *)definition {
    return [name compare:[definition name]];
}

- (NSComparisonResult)compareByUsageCount:(EntityDefinition *)definition {
    if (usageCount > [definition usageCount])
        return NSOrderedAscending;
    if (usageCount < [definition usageCount])
        return NSOrderedDescending;
    return [self compareByName:definition];
}

- (void)dealloc {
    [name release];
    [flags release];
    [properties release];
    [description release];
    [super dealloc];
}

@end

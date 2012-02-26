/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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

- (id)initPointDefinitionWithName:(NSString *)theName color:(TVector4f *)theColor bounds:(TBoundingBox *)theBounds flags:(NSDictionary *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theColor != NULL, @"color must not be null");
    NSAssert(theBounds != nil, @"bounds must not be nil");
    
    if ((self = [self init])) {
        name = [theName retain];
        color = *theColor;
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

- (id)initBrushDefinitionWithName:(NSString *)theName color:(TVector4f *)theColor flags:(NSDictionary *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theColor != NULL, @"color must not be null");
    
    if ((self = [self init])) {
        name = [theName retain];
        color = *theColor;
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

- (const TVector4f *)color {
    return &color;
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
    
    for (SpawnFlag* flag in [flags objectEnumerator])
        if ((theMask & [flag flag]) != 0)
            [result addObject:flag];
    
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
    
    for (id <EntityDefinitionProperty> property in properties) {
        if ([property isKindOfClass:[ModelProperty class]]) {
            ModelProperty* modelProperty = (ModelProperty *)property;
            NSString* flagName = [modelProperty flagName];
            
            if (flagName == nil) {
                defaultProperty = modelProperty;
            } else if ([self isFlag:flagName setOnEntity:theEntity]) {
                specificProperty = modelProperty;
                break;
            }
        }
    }
    
    return specificProperty != nil ? specificProperty : defaultProperty;
}

- (ModelProperty *)defaultModelProperty {
    for (id <EntityDefinitionProperty> property in properties) {
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

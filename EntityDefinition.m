//
//  EntityDefinition.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinition.h"

@implementation EntityDefinition

- (id)initBaseDefinitionWithName:(NSString *)theName flags:(NSArray *)theFlags properties:(NSArray *)theProperties {
    NSAssert(theName != nil, @"name must not be nil");
    
    if (self = [self init]) {
        name = [theName retain];
        flags = [theFlags retain];
        properties = [theProperties retain];
        type = EDT_BASE;
    }
    
    return self;
}

- (id)initPointDefinitionWithName:(NSString *)theName color:(float *)theColor bounds:(TBoundingBox *)theBounds flags:(NSArray *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theColor != NULL, @"color must not be null");
    NSAssert(theBounds != nil, @"bounds must not be nil");
    
    if (self = [self init]) {
        name = [theName retain];
        memcpy(color, theColor, 3 * sizeof(float));
        bounds = *theBounds;
        flags = [theFlags retain];
        properties = [theProperties retain];
        description = [theDescription retain];
        type = EDT_POINT;
    }
    
    return self;
}

- (id)initBrushDefinitionWithName:(NSString *)theName color:(float *)theColor flags:(NSArray *)theFlags properties:(NSArray *)theProperties description:(NSString *)theDescription {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theColor != NULL, @"color must not be null");
    
    if (self = [self init]) {
        name = [theName retain];
        memcpy(color, theColor, 3 * sizeof(float));
        flags = [theFlags retain];
        properties = [theProperties retain];
        description = [theDescription retain];
        type = EDT_BRUSH;
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

- (TBoundingBox *)bounds {
    return &bounds;
}

- (NSArray *)flags {
    return flags;
}

- (NSArray *)properties {
    return properties;
}

- (NSString *)description {
    return description;
}

- (NSComparisonResult)compareByName:(EntityDefinition *)definition {
    return [name compare:[definition name]];
}

- (void)dealloc {
    [name release];
    [flags release];
    [properties release];
    [description release];
    [super dealloc];
}

@end

//
//  Prefab.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Prefab.h"
#import "Map.h"
#import "Brush.h"
#import "MutableEntity.h"
#import "MutableBrush.h"
#import "Vector3i.h"
#import "Vector3f.h"

@implementation Prefab

- (id)init {
    if (self = [super init]) {
        entities = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (NSArray *)entities {
    return entities;
}

- (void)addEntity:(MutableEntity *)theEntity {
    [entities addObject:theEntity];
    [theEntity setMap:self];
    
    [center release];
    center = nil;
    [bounds release];
    bounds = nil;
}

- (void)removeEntity:(MutableEntity *)theEntity {
    [theEntity setMap:nil];
    [entities removeObject:theEntity];
    
    [center release];
    center = nil;
    [bounds release];
    bounds = nil;
}

- (BoundingBox *)bounds {
    if (bounds == nil && [entities count] > 0) {
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity = [entityEn nextObject];

        bounds = [[BoundingBox alloc] initWithMin:[[entity bounds] min] max:[[entity bounds] max]];
        while ((entity = [entityEn nextObject]))
            [bounds merge:[entity bounds]];
    }
    
    return bounds;
}

- (Vector3f *)center {
    if (center == nil && [entities count] > 0) {
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity = [entityEn nextObject];
        
        center = [[Vector3f alloc] initWithFloatVector:[entity center]];
        while ((entity = [entityEn nextObject]))
            [center add:[entity center]];
        
        [center scale:1.0f / [entities count]];
    }
    
    return center;
}

- (void)translateToOrigin {
    if ([entities count] == 0)
        return;
    
    Vector3i* offset = [[Vector3i alloc] init];
    [offset setX:-[[self center] x]];
    [offset setY:-[[self center] y]];
    [offset setZ:-[[self center] z]];
    
    [center release];
    
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            MutableBrush* mutableBrush = (MutableBrush *)brush;
            [mutableBrush translateBy:offset];
        }
    }
    
    [offset release];
}

- (void)dealloc {
    [entities release];
    [bounds release];
    [center release];
    [super dealloc];
}

@end

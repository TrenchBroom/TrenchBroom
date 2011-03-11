//
//  Prefab.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MutablePrefab.h"
#import "Map.h"
#import "Brush.h"
#import "MutableEntity.h"
#import "MutableBrush.h"
#import "Vector3i.h"
#import "Vector3f.h"
#import "MathCache.h"
#import "IdGenerator.h"
#import "MutablePrefabGroup.h"
#import "MapDocument.h"

@implementation MutablePrefab

- (id)init {
    if (self = [super init]) {
        prefabId = [[[IdGenerator sharedGenerator] getId] retain];
        entities = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)theName group:(MutablePrefabGroup *)thePrefabGroup readOnly:(BOOL)isReadOnly {
    if (self = [self init]) {
        name = [theName retain];
        prefabGroup = [thePrefabGroup retain];
        readOnly = isReadOnly;
    }
    
    return self;
}

- (NSString *)name {
    return name;
}

- (id <PrefabGroup>)prefabGroup {
    return prefabGroup;
}

- (NSNumber *)prefabId {
    return prefabId;
}

- (BOOL)readOnly {
    return readOnly;
}

- (id <Entity>)worldspawn {
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        if ([entity isWorldspawn])
            return entity;
    
    return nil;
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
    [maxBounds release];
    maxBounds = nil;
}

- (void)removeEntity:(MutableEntity *)theEntity {
    [theEntity setMap:nil];
    [entities removeObject:theEntity];
    
    [center release];
    center = nil;
    [bounds release];
    bounds = nil;
    [maxBounds release];
    maxBounds = nil;
}

- (BoundingBox *)bounds {
    if (bounds == nil && [entities count] > 0) {
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity = [entityEn nextObject];

        bounds = [[BoundingBox alloc] initWithBounds:[entity bounds]];
        while ((entity = [entityEn nextObject]))
            [bounds merge:[entity bounds]];
    }
    
    return bounds;
}

- (BoundingBox *)maxBounds {
    if (maxBounds == nil && [entities count] > 0) {
        MathCache* cache = [MathCache sharedCache];
        Vector3f* diff = [cache vector3f];
        
        float distSquared = 0;
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
            id <Brush> brush;
            while ((brush = [brushEn nextObject])) {
                NSEnumerator* vertexEn = [[brush vertices] objectEnumerator];
                Vector3f* vertex;
                while ((vertex = [vertexEn nextObject])) {
                    [diff setFloat:vertex];
                    [diff sub:[self center]];
                    float lengthSquared = [diff lengthSquared];
                    if (lengthSquared > distSquared)
                        distSquared = lengthSquared;
                }
            }
        }
        
        [cache returnVector3f:diff];
        
        if (distSquared > 0) {
            float dist = sqrt(distSquared);
            MathCache* cache = [MathCache sharedCache];
            Vector3f* min = [cache vector3f];
            Vector3f* max = [cache vector3f];
            
            [min setX:-dist];
            [min setY:-dist];
            [min setZ:-dist];
            [max setX:dist];
            [max setY:dist];
            [max setZ:dist];
            
            [min add:[self center]];
            [max add:[self center]];
            
            maxBounds = [[BoundingBox alloc] initWithMin:min max:max];
            
            [cache returnVector3f:min];
            [cache returnVector3f:max];
        }
    }
    
    return maxBounds;
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

- (void)setPrefabGroup:(MutablePrefabGroup *)thePrefabGroup {
    [prefabGroup release];
    prefabGroup = [thePrefabGroup retain];
}

- (NSComparisonResult)compareByName:(MutablePrefab *)prefab {
    return [name localizedCaseInsensitiveCompare:[prefab name]];
}

- (void)dealloc {
    [prefabGroup release];
    [prefabId release];
    [entities release];
    [bounds release];
    [maxBounds release];
    [center release];
    [name release];
    [super dealloc];
}

@end

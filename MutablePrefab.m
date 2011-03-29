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
#import "Vertex.h"
#import "Vector3i.h"
#import "Vector3f.h"
#import "BoundingBox.h"
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

- (id <Entity>)worldspawn:(BOOL)create {
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        if ([entity isWorldspawn])
            return entity;
    
    if (create) {
        entity = [[MutableEntity alloc] initWithProperties:[NSDictionary dictionaryWithObject:@"worldspawn" forKey:@"classname"]];
        [self addEntity:entity];
        return [entity autorelease];
    }
    
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
            [bounds mergeBounds:[entity bounds]];
    }
    
    return bounds;
}

- (BoundingBox *)maxBounds {
    if (maxBounds == nil && [entities count] > 0) {
        Vector3f* diff = [[Vector3f alloc] init];
        
        float distSquared = 0;
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
            id <Brush> brush;
            while ((brush = [brushEn nextObject])) {
                NSEnumerator* vertexEn = [[brush vertices] objectEnumerator];
                Vertex* vertex;
                while ((vertex = [vertexEn nextObject])) {
                    [diff setFloat:[vertex vector]];
                    [diff sub:[self center]];
                    float lengthSquared = [diff lengthSquared];
                    if (lengthSquared > distSquared)
                        distSquared = lengthSquared;
                }
            }
        }

        [diff release];
        
        if (distSquared > 0) {
            float dist = sqrt(distSquared);
            Vector3f* min = [[Vector3f alloc] initWithFloatX:-dist y:-dist z:-dist];
            Vector3f* max = [[Vector3f alloc] initWithFloatX:dist y:dist z:dist];
            
            [min add:[self center]];
            [max add:[self center]];
            
            maxBounds = [[BoundingBox alloc] initWithMin:min max:max];
            [min release];
            [max release];
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

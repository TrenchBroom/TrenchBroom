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

#import "MutablePrefab.h"
#import "Map.h"
#import "Brush.h"
#import "MutableEntity.h"
#import "MutableBrush.h"
#import "IdGenerator.h"
#import "MutablePrefabGroup.h"
#import "MapDocument.h"
#import "VertexData.h"

@interface MutablePrefab (private)

- (void)validate;

@end

@implementation MutablePrefab (private)

- (void)validate {
    
    if ([entities count] > 0) {
        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity = [entityEn nextObject];
        
        bounds = *[entity bounds];
        center = *[entity center];
        
        while ((entity = [entityEn nextObject])) {
            mergeBoundsWithBounds(&bounds, [entity bounds], &bounds);
            addV3f(&center, [entity center], &center);
        }
        scaleV3f(&center, 1.0f / [entities count], &center);

        TVector3f diff;
        float distSquared = 0;
        
        entityEn = [entities objectEnumerator];
        while ((entity = [entityEn nextObject])) {
            NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
            id <Brush> brush;
            while ((brush = [brushEn nextObject])) {
                const TVertexList* vertices = [brush vertices];
                for (int i = 0; i < vertices->count; i++) {
                    subV3f(&vertices->items[i]->position, &center, &diff);
                    float lengthSquared = lengthSquaredV3f(&diff);
                    if (lengthSquared > distSquared)
                        distSquared = lengthSquared;
                }
            }
        }
        
        if (distSquared > 0) {
            float dist = sqrt(distSquared);
            maxBounds.min.x = -dist;
            maxBounds.min.y = -dist;
            maxBounds.min.z = -dist;
            maxBounds.max.x = +dist;
            maxBounds.max.y = +dist;
            maxBounds.max.z = +dist;

            addV3f(&maxBounds.min, &center, &maxBounds.min);
            addV3f(&maxBounds.max, &center, &maxBounds.max);
        } else {
            maxBounds.min = NullVector;
            maxBounds.max = NullVector;
        }
    } else {
        bounds.min = NullVector;
        bounds.max = NullVector;
        maxBounds.min = NullVector;
        maxBounds.max = NullVector;
        center = NullVector;
    }

    valid = YES;
}

@end

@implementation MutablePrefab

- (id)initWithWorldBounds:(TBoundingBox *)theWorldBounds name:(NSString *)theName group:(MutablePrefabGroup *)thePrefabGroup readOnly:(BOOL)isReadOnly {
    if ((self = [self init])) {
        worldBounds = theWorldBounds;
        name = [theName retain];
        prefabGroup = [thePrefabGroup retain];
        readOnly = isReadOnly;
        prefabId = [[[IdGenerator sharedGenerator] getId] retain];
        entities = [[NSMutableArray alloc] init];
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

- (TBoundingBox *)worldBounds {
    return worldBounds;
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

- (void)addEntities:(NSArray *)theEntities {
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        [entities addObject:entity];
        [entity setMap:self];
    }
    
    valid = NO;
}

- (void)addEntity:(MutableEntity *)theEntity {
    [entities addObject:theEntity];
    [theEntity setMap:self];

    valid = NO;
}

- (void)removeEntities:(NSArray *)theEntities {
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    MutableEntity* entity;
    while ((entity = [entityEn nextObject])) {
        [entities removeObject:entity];
        [entity setMap:nil];
    }
    
    valid = NO;
}

- (void)removeEntity:(MutableEntity *)theEntity {
    [theEntity setMap:nil];
    [entities removeObject:theEntity];

    valid = NO;
}

- (TBoundingBox *)bounds {
    if (!valid)
        [self validate];
    
    return &bounds;
}

- (TBoundingBox *)maxBounds {
    if (!valid)
        [self validate];
    
    return &maxBounds;
}

- (TVector3f *)center {
    if (!valid)
        [self validate];
    
    return &center;
}

- (void)setName:(NSString *)theName {
    [name release];
    name = [theName retain];
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
    [name release];
    [super dealloc];
}

@end

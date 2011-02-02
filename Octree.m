//
//  Octree.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Octree.h"
#import "OctreeNode.h"
#import "Vector3i.h"
#import "Ray3D.h"
#import "Map.h"
#import "Entity.h"
#import "Brush.h"

@implementation Octree

- (void)addEntity:(Entity *)entity {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(brushAdded:) name:EntityBrushAdded object:entity];
    [center addObserver:self selector:@selector(brushRemoved:) name:EntityBrushRemoved object:entity];
}

- (void)entityAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Entity* entity = [userInfo objectForKey:MapEntityKey];
    [self addEntity:entity];
}

- (void)entityRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Entity* entity = [userInfo objectForKey:MapEntityKey];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:EntityBrushAdded object:entity];
    [center removeObserver:self name:EntityBrushRemoved object:entity];
}

- (void)addBrush:(Brush *)brush {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(brushChanged:) name:BrushGeometryChanged object:brush];
    [root addObject:brush bounds:[brush bounds]];
}

- (void)brushAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Brush* brush = [userInfo objectForKey:EntityBrushKey];
    [self addBrush:brush];
}

- (void)brushRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Brush* brush = [userInfo objectForKey:EntityBrushKey];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:BrushGeometryChanged object:brush];
    [root removeObject:brush bounds:[brush bounds]];
}

- (void)brushChanged:(NSNotification *)notification {
    Brush* brush = [notification object];

    [root removeObject:brush bounds:[brush bounds]];
    [root addObject:brush bounds:[brush bounds]];
}

- (id)initWithMap:(Map *)theMap minSize:(int)theMinSize {
    if (theMap == nil)
        [NSException raise:NSInvalidArgumentException format:@"map must not be nil"];
    
    if (self = [self init]) {
        map = [theMap retain];
        
        int w = [map worldSize] / 2;
        Vector3i* min = [[Vector3i alloc] initWithX:-w y:-w z:-w];
        Vector3i* max = [[Vector3i alloc] initWithX:+w y:+w z:+w];

        root = [[OctreeNode alloc] initWithMin:min max:max minSize:theMinSize];
        
        [min release];
        [max release];
        
        NSArray* entities = [map entities];
        NSEnumerator* entityEn = [entities objectEnumerator];
        Entity* entity;
        while ((entity = [entityEn nextObject])) {
            NSArray* brushes = [entity brushes];
            NSEnumerator* brushEn = [brushes objectEnumerator];
            Brush* brush;
            while ((brush = [brushEn nextObject]))
                [self addBrush:brush];
            
            [self addEntity:entity];
        }
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(entityAdded:) name:MapEntityAdded object:map];
        [center addObserver:self selector:@selector(entityRemoved:) name:MapEntityRemoved object:map];
    }
    
    return self;
}

- (void)addObjectsForRay:(Ray3D *)theRay to:(NSMutableSet *)theSet {
    [root addObjectsForRay:theRay to:theSet];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [root release];
    [map release];
    [super dealloc];
}
@end

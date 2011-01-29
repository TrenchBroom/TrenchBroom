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
            while ((brush = [brushEn nextObject])) {
                [root addObject:brush bounds:[brush bounds]];
            }
        }
    }
    
    return self;
}

- (void)addObjectsForRay:(Ray3D *)theRay to:(NSMutableSet *)theSet {
    [root addObjectsForRay:theRay to:theSet];
}

- (void)dealloc {
    [root release];
    [map release];
    [super dealloc];
}
@end

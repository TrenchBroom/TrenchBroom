//
//  PickingManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Picker.h"
#import "Octree.h"
#import "Ray3D.h"
#import "Brush.h"
#import "PickingHit.h"

@implementation Picker
- (id)initWithOctree:(Octree *)theOctree {
    if (theOctree == nil)
        [NSException raise:NSInvalidArgumentException format:@"octree must not be nil"];
    
    if (self = [self init]) {
        octree = [theOctree retain];
    }
    
    return self;
}

- (NSArray *)objectsHitByRay:(Ray3D *)theRay {
    if (theRay == nil)
        [NSException raise:NSInvalidArgumentException format:@"ray must not be nil"];
    
    NSMutableArray* hits = [[NSMutableArray alloc] init];
    
    NSMutableSet* objects = [[NSMutableSet alloc] init];
    [octree addObjectsForRay:theRay to:objects];
    
    NSLog(@"%i candidates", [objects count]);
    
    NSEnumerator* objectEn = [objects objectEnumerator];
    id object;
    while ((object = [objectEn nextObject])) {
        if ([object isKindOfClass:[Brush class]]) {
            Brush* brush = (Brush *)object;
            PickingHit* hit = [brush pickFace:theRay];
            if (hit != nil)
                [hits addObject:hit];
        }
    }
    
    [hits sortUsingSelector:@selector(compareTo:)];
    return hits;
}

- (void)dealloc {
    [octree release];
    [super dealloc];
}
@end

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

- (id)initWithDocument:(MapDocument *)theDocument {
    if (theDocument == nil)
        [NSException raise:NSInvalidArgumentException format:@"document must not be nil"];
    
    if (self = [self init]) {
        octree = [[Octree alloc] initWithDocument:theDocument minSize:32];
    }
    
    return self;
}

- (NSArray *)objectsHitByRay:(Ray3D *)theRay {
    if (theRay == nil)
        [NSException raise:NSInvalidArgumentException format:@"ray must not be nil"];
    
    NSMutableArray* hits = [[NSMutableArray alloc] init];
    
    NSMutableSet* objects = [[NSMutableSet alloc] init];
    [octree addObjectsForRay:theRay to:objects];
    
    NSEnumerator* objectEn = [objects objectEnumerator];
    id object;
    while ((object = [objectEn nextObject])) {
        if ([object conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)object;
            PickingHit* hit = [brush pickFace:theRay];
            if (hit != nil)
                [hits addObject:hit];
        }
    }
    [objects release];
    
    [hits sortUsingSelector:@selector(compareTo:)];
    return [hits autorelease];
}

- (void)dealloc {
    [octree release];
    [super dealloc];
}
@end

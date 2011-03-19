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
#import "PickingHitList.h"

@implementation Picker

- (id)initWithDocument:(MapDocument *)theDocument {
    if (theDocument == nil)
        [NSException raise:NSInvalidArgumentException format:@"document must not be nil"];
    
    if (self = [self init]) {
        octree = [[Octree alloc] initWithDocument:theDocument minSize:32];
    }
    
    return self;
}

- (PickingHitList *)pickObjects:(Ray3D *)theRay include:(NSSet *)includedObjects exclude:(NSSet *)excludedObjects {
    if (theRay == nil)
        [NSException raise:NSInvalidArgumentException format:@"ray must not be nil"];
    
    PickingHitList* hitList = [[PickingHitList alloc] init];
    
    NSMutableSet* objects = [[NSMutableSet alloc] init];
    [octree addObjectsForRay:theRay to:objects];
    
    if (includedObjects != nil)
        [objects intersectSet:includedObjects];
    if (excludedObjects != nil)
        [objects minusSet:excludedObjects];
    
    NSEnumerator* objectEn = [objects objectEnumerator];
    id object;
    while ((object = [objectEn nextObject])) {
        if ([object conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)object;
            [brush pickBrush:theRay hitList:hitList];
            [brush pickFace:theRay hitList:hitList];
            [brush pickEdge:theRay hitList:hitList];
            [brush pickVertex:theRay hitList:hitList];
        }
    }
    [objects release];
    
    return [hitList autorelease];
}

- (void)dealloc {
    [octree release];
    [super dealloc];
}
@end

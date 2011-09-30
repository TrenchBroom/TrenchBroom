//
//  PickingManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Picker.h"
#import "Octree.h"
#import "Entity.h"
#import "Brush.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Filter.h"

@implementation Picker

- (id)initWithMap:(MapDocument *)theMap {
    if ((self = [self init])) {
        octree = [[Octree alloc] initWithMap:theMap minSize:64];
    }
    
    return self;
}

- (PickingHitList *)pickObjects:(TRay *)ray filter:(id <Filter>)filter {
    PickingHitList* hitList = [[PickingHitList alloc] init];
    NSArray* objects = [octree pickObjectsWithRay:ray];
    
    NSEnumerator* objectEn = [objects objectEnumerator];
    id object;
    while ((object = [objectEn nextObject])) {
        if ([object conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)object;
            if (filter == nil || [filter isBrushPickable:brush])
                [brush pick:ray hitList:hitList];
        } else if ([object conformsToProtocol:@protocol(Entity)]) {
            id <Entity> entity = (id <Entity>)object;
            if (filter == nil || [filter isEntityPickable:entity])
                [entity pick:ray hitList:hitList];
        }
    }

    return [hitList autorelease];
}

- (void)dealloc {
    [octree release];
    [super dealloc];
}
@end

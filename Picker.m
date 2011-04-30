//
//  PickingManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Picker.h"
#import "Octree.h"
#import "Brush.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "Filter.h"

@implementation Picker

- (id)initWithDocument:(MapDocument *)theDocument {
    if (self = [self init]) {
        octree = [[Octree alloc] initWithDocument:theDocument minSize:64];
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
            if (filter == nil || [filter brushPasses:brush])
                [brush pick:ray hitList:hitList];
        }
    }

    return [hitList autorelease];
}

- (void)dealloc {
    [octree release];
    [super dealloc];
}
@end

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
    if (self = [self init]) {
        octree = [[Octree alloc] initWithDocument:theDocument minSize:32];
    }
    
    return self;
}

- (PickingHitList *)pickObjects:(Ray3D *)ray include:(NSSet *)include exclude:(NSSet *)exclude {

    PickingHitList* hitList = [[PickingHitList alloc] init];
    NSArray* objects = [octree pickObjectsWithRay:ray include:include exclude:exclude];
    
    NSEnumerator* objectEn = [objects objectEnumerator];
    id object;
    while ((object = [objectEn nextObject])) {
        if ([object conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = (id <Brush>)object;
            [brush pickBrush:ray hitList:hitList];
            [brush pickFace:ray hitList:hitList];
//            [brush pickEdge:theRay hitList:hitList];
//            [brush pickVertex:theRay hitList:hitList];
        }
    }
    
    return [hitList autorelease];
}

- (void)dealloc {
    [octree release];
    [super dealloc];
}
@end

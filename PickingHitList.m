//
//  PickingHitList.m
//  TrenchBroom
//
//  Created by Kristian Duske on 19.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "PickingHitList.h"

@implementation PickingHitList

- (id)init {
    if (self = [super init]) {
        hitList = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)addHit:(PickingHit *)theHit {
    [hitList addObject:theHit];
    sorted = NO;
}

- (PickingHit *)firstHitOfType:(EHitType)theTypeMask ignoreOccluders:(BOOL)ignoreOccluders {
    if ([hitList count] == 0)
        return nil;
    
    if (!ignoreOccluders) {
        PickingHit* hit = [[self hits] objectAtIndex:0];
        if ([hit isType:theTypeMask])
            return hit;
    } else {
        NSEnumerator* hitEn = [[self hits] objectEnumerator];
        PickingHit* hit;
        while ((hit = [hitEn nextObject]))
            if ([hit isType:theTypeMask])
                return hit;
    }

    return nil;
}

- (NSArray *)hitsOfType:(EHitType)theTypeMask {
    if ([hitList count] == 0)
        return [NSArray array];
    
    NSMutableArray* result = [[NSMutableArray alloc] init];
    
    NSEnumerator* hitEn = [[self hits] objectEnumerator];
    PickingHit* hit;
    while ((hit = [hitEn nextObject]))
        if ([hit isType:theTypeMask])
            [result addObject:hit];
    
    
    return [result autorelease];
}

- (NSSet *)objectsOfType:(EHitType)theTypeMask {
    if ([hitList count] == 0)
        return [NSSet set];
    
    NSMutableSet* result = [[NSMutableSet alloc] init];
    
    NSEnumerator* hitEn = [[self hits] objectEnumerator];
    PickingHit* hit;
    while ((hit = [hitEn nextObject]))
        if ([hit isType:theTypeMask])
            [result addObject:[hit object]];
    
    
    return [result autorelease];
}

- (NSArray *)hits {
    if (!sorted) {
        [hitList sortUsingSelector:@selector(compareTo:)];
        sorted = YES;
    }
    
    return hitList;
}

- (void)dealloc {
    [hitList release];
    [super dealloc];
}

@end

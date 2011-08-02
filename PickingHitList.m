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
        for (int i = 0; i < HT_ANY; i++) {
            cachedHitValid[NO][i] = NO;
            cachedHitValid[YES][i] = NO;
            cachedHitListValid[i] = NO;
            cachedObjectSetValid[i] = NO;
            cachedHit[NO][i] = nil;
            cachedHit[YES][i] = nil;
            cachedHitList[i] = nil;
            cachedObjectSet[i] = nil;
        }
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
    
    if (!cachedHitValid[ignoreOccluders][theTypeMask - 1]) {
        if (!ignoreOccluders) {
            PickingHit* hit = [[self hits] objectAtIndex:0];
            if ([hit isType:theTypeMask])
                cachedHit[NO][theTypeMask - 1] = hit;
            cachedHitValid[NO][theTypeMask - 1] = YES;
        } else {
            NSEnumerator* hitEn = [[self hits] objectEnumerator];
            PickingHit* hit;
            while ((hit = [hitEn nextObject]))
                if ([hit isType:theTypeMask]) {
                    cachedHit[YES][theTypeMask - 1] = hit;
                    break;
                }
            cachedHitValid[YES][theTypeMask - 1] = YES;
        }
    }
    
    return cachedHit[ignoreOccluders][theTypeMask - 1];
}

- (NSArray *)hitsOfType:(EHitType)theTypeMask {
    if ([hitList count] == 0)
        return [NSArray array];
    
    if (!cachedHitListValid[theTypeMask - 1]) {
        NSMutableArray* result = [[NSMutableArray alloc] init];
        
        NSEnumerator* hitEn = [[self hits] objectEnumerator];
        PickingHit* hit;
        while ((hit = [hitEn nextObject]))
            if ([hit isType:theTypeMask])
                [result addObject:hit];
        
        cachedHitList[theTypeMask - 1] = result;
        cachedHitListValid[theTypeMask - 1] = YES;
    }
    
    return cachedHitList[theTypeMask - 1];
}

- (NSSet *)objectsOfType:(EHitType)theTypeMask {
    if ([hitList count] == 0)
        return [NSSet set];
    
    if (!cachedObjectSetValid[theTypeMask - 1]) {
        NSMutableSet* result = [[NSMutableSet alloc] init];
        
        NSEnumerator* hitEn = [[self hitsOfType:theTypeMask] objectEnumerator];
        PickingHit* hit;
        while ((hit = [hitEn nextObject]))
            if ([hit isType:theTypeMask])
                [result addObject:[hit object]];

        cachedObjectSet[theTypeMask - 1] = result;
        cachedObjectSetValid[theTypeMask - 1] = YES;
    }
    
    return cachedObjectSet[theTypeMask - 1];
    
}

- (NSArray *)hits {
    if (!sorted) {
        [hitList sortUsingSelector:@selector(compareTo:)];
        sorted = YES;
    }
    
    return hitList;
}

- (void)dealloc {
    for (int i = 0; i < HT_ANY; i++) {
        [cachedHitList[i] release];
        [cachedObjectSet[i] release];
    }
    [hitList release];
    [super dealloc];
}

@end

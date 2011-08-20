//
//  PickingHitList.m
//  TrenchBroom
//
//  Created by Kristian Duske on 19.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "PickingHitList.h"
#import "Brush.h"
#import "Face.h"

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
    
    PickingHit* hit = nil;
    if (!ignoreOccluders) {
        hit = [[self hits] objectAtIndex:0];
        if (![hit isType:theTypeMask])
            hit = nil;
    } else {
        NSEnumerator* hitEn = [[self hits] objectEnumerator];
        while ((hit = [hitEn nextObject]) && ![hit isType:theTypeMask]);
    }
    
    return hit;
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
    
    NSEnumerator* hitEn = [[self hitsOfType:theTypeMask] objectEnumerator];
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

- (PickingHit *)edgeDragHit {
    PickingHit* firstHit = [self firstHitOfType:HT_CLOSE_EDGE ignoreOccluders:NO];
    if (firstHit == nil)
        return nil;
    
    NSArray* hits = [self hitsOfType:HT_CLOSE_EDGE];
    if ([hits count] == 1)
        return firstHit;
    
    id <Face> firstHitFace = [firstHit object];
    id <Brush> firstHitBrush = [firstHitFace brush];
    
    NSEnumerator* hitEn = [hits objectEnumerator];
    PickingHit* hit;
    while ((hit = [hitEn nextObject])) {
        id <Face> face = [hit object];
        id <Brush> brush = [face brush];
        if (hit != firstHit && brush != firstHitBrush)
            return nil;
    }
    
    return firstHit;
}

- (void)dealloc {
    [hitList release];
    [super dealloc];
}

@end

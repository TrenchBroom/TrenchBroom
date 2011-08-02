//
//  PickingHitList.h
//  TrenchBroom
//
//  Created by Kristian Duske on 19.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "PickingHit.h"

@interface PickingHitList : NSObject {
    @private
    NSMutableArray* hitList;
    BOOL sorted;
    BOOL cachedHitValid[2][HT_ANY];
    BOOL cachedHitListValid[HT_ANY];
    BOOL cachedObjectSetValid[HT_ANY];
    PickingHit* cachedHit[2][HT_ANY];
    NSArray* cachedHitList[HT_ANY];
    NSSet* cachedObjectSet[HT_ANY];
}

- (void)addHit:(PickingHit *)theHit;
- (PickingHit *)firstHitOfType:(EHitType)theTypeMask ignoreOccluders:(BOOL)ignoreOccluders;
- (NSArray *)hitsOfType:(EHitType)theTypeMask;
- (NSSet *)objectsOfType:(EHitType)theTypeMask;
- (NSArray *)hits;

@end

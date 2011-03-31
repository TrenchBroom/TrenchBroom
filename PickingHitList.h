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
}

- (void)addHit:(PickingHit *)theHit;
- (PickingHit *)firstHitOfType:(EHitType)theTypeMask ignoreOccluders:(BOOL)ignoreOccluders;
- (NSArray *)hitsOfType:(EHitType)theTypeMask;
- (NSSet *)objectsOfType:(EHitType)theTypeMask;
- (NSArray *)hits;

@end

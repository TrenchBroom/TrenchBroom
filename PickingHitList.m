/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    
    if (!ignoreOccluders) {
        NSEnumerator* hitEn = [[self hits] objectEnumerator];
        PickingHit* hit = [hitEn nextObject];
        if ([hit isType:theTypeMask])
            return hit;
        
        float dist = [hit distance];
        while ((hit = [hitEn nextObject]) && ![hit isType:theTypeMask] && [hit distance] == dist);
        
        if ([hit distance] == dist)
            return hit;
    } else {
        NSEnumerator* hitEn = [[self hits] objectEnumerator];
        PickingHit* hit;
        while ((hit = [hitEn nextObject]) && ![hit isType:theTypeMask]);
        return hit;
    }
    
    return nil;
}

- (NSArray *)hitsOfType:(EHitType)theTypeMask {
    if ([hitList count] == 0)
        return [NSArray array];
    
    NSMutableArray* result = [[NSMutableArray alloc] init];
    
    for (PickingHit* hit in [self hits])
        if ([hit isType:theTypeMask])
            [result addObject:hit];
    
    return [result autorelease];
}

- (NSSet *)objectsOfType:(EHitType)theTypeMask {
    if ([hitList count] == 0)
        return [NSSet set];
    
    NSMutableSet* result = [[NSMutableSet alloc] init];
    
    for (PickingHit* hit in [self hits])
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

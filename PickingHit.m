//
//  PickingHit.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PickingHit.h"
#import "Vector3f.h"

@implementation PickingHit

- (id)initWithObject:(id)theObject type:(EHitType)theType hitPoint:(Vector3f *)theHitPoint distance:(float)theDistance {
    if (theObject == nil)
        [NSException raise:NSInvalidArgumentException format:@"object must not be nil"];
    if (theHitPoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"hit point must not be nil"];
    
    if (self = [self init]) {
        object = [theObject retain];
        type = theType;
        hitPoint = [theHitPoint retain];
        distance = theDistance;
    }
    
    return self;
}

- (id)object {
    return object;
}

- (EHitType)type {
    return type;
}

- (BOOL)isType:(EHitType)theTypeMask {
    return (theTypeMask & type) != 0;
}

- (Vector3f *)hitPoint {
    return hitPoint;
}

- (float)distance {
    return distance;
}

- (NSComparisonResult)compareTo:(PickingHit *)other {
    if (distance < [other distance])
        return NSOrderedAscending;
    if (distance > [other distance])
        return NSOrderedDescending;
    return NSOrderedSame;
}

- (void)dealloc {
    [object release];
    [hitPoint release];
    [super dealloc];
}

@end

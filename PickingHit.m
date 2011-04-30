//
//  PickingHit.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PickingHit.h"

@implementation PickingHit

- (id)initWithObject:(id)theObject type:(EHitType)theType hitPoint:(TVector3f *)theHitPoint distance:(float)theDistance {
    if (self = [self init]) {
        object = [theObject retain];
        type = theType;
        hitPoint = *theHitPoint;
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

- (TVector3f *)hitPoint {
    return &hitPoint;
}

- (float)distance {
    return distance;
}

- (NSComparisonResult)compareTo:(PickingHit *)other {
    if (distance < [other distance])
        return NSOrderedAscending;
    if (distance > [other distance])
        return NSOrderedDescending;
    if (type == HT_FACE && [other type] == HT_BRUSH)
        return NSOrderedAscending;
    if (type == HT_BRUSH && [other type] == HT_FACE)
        return NSOrderedDescending;
    return NSOrderedSame;
}

- (void)dealloc {
    [object release];
    [super dealloc];
}

@end

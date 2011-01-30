//
//  PickingHit.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface PickingHit : NSObject {
    id object;
    Vector3f* hitPoint;
    float distance;
}

+ (PickingHit *)hitWithObject:(id)theObject hitPoint:(Vector3f *)theHitPoint distance:(float)theDistance;

- (id)initWithObject:(id)theObject hitPoint:(Vector3f *)theHitPoint distance:(float)theDistance;

- (id)object;
- (Vector3f *)hitPoint;
- (float)distance;

- (NSComparisonResult)compareTo:(PickingHit *)other;

@end

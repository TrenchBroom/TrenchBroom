//
//  PickingHit.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    HT_BRUSH  = 1 << 0,
    HT_FACE   = 1 << 1,
    HT_EDGE   = 1 << 2,
    HT_VERTEX = 1 << 3
} EHitType;

@class Vector3f;

@interface PickingHit : NSObject {
    id object;
    EHitType type;
    Vector3f* hitPoint;
    float distance;
}

- (id)initWithObject:(id)theObject type:(EHitType)theType hitPoint:(Vector3f *)theHitPoint distance:(float)theDistance;

- (id)object;
- (EHitType)type;
- (BOOL)isType:(EHitType)theTypeMask;
- (Vector3f *)hitPoint;
- (float)distance;

- (NSComparisonResult)compareTo:(PickingHit *)other;

@end

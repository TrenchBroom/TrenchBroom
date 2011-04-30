//
//  PickingHit.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    HT_ENTITY = 1 << 0,
    HT_BRUSH  = 1 << 1,
    HT_FACE   = 1 << 2,
    HT_EDGE   = 1 << 3,
    HT_VERTEX = 1 << 4,
    HT_ANY    = HT_ENTITY | HT_BRUSH | HT_FACE | HT_EDGE | HT_VERTEX
} EHitType;

@interface PickingHit : NSObject {
    id object;
    EHitType type;
    TVector3f hitPoint;
    float distance;
}

- (id)initWithObject:(id)theObject type:(EHitType)theType hitPoint:(TVector3f *)theHitPoint distance:(float)theDistance;

- (id)object;
- (EHitType)type;
- (BOOL)isType:(EHitType)theTypeMask;
- (TVector3f *)hitPoint;
- (float)distance;

- (NSComparisonResult)compareTo:(PickingHit *)other;

@end

//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@protocol Entity;
@class PickingHit;
@class PickingHitList;
@protocol Face;

@protocol Brush <NSObject, NSCopying>

- (NSNumber* )brushId;
- (id <Entity>)entity;
- (id)copy;

- (NSArray *)faces;
- (NSArray *)vertices;
- (NSArray *)edges;

- (float *)flatColor;
- (TBoundingBox *)bounds;
- (TVector3f *)center;

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList;
- (float)pickHotFace:(TRay *)theRay maxDistance:(float)theMaxDistance hit:(id <Face> *)theHit;

@end

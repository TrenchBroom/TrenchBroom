//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class PickingHit;
@class PickingHitList;
@protocol Entity;
@protocol Face;

@protocol Brush <NSObject, NSCopying>

- (NSNumber* )brushId;
- (id <Entity>)entity;
- (id)copy;

- (NSArray *)faces;
- (NSArray *)vertices;
- (NSArray *)edges;

- (TBoundingBox *)bounds;
- (TVector3f *)center;

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList;
- (void)pickEdgeClosestToRay:(TRay *)theRay maxDistance:(float)theMaxDist hitList:(PickingHitList *)theHitList;

- (BOOL)intersectsBrush:(id <Brush>)theBrush;
- (BOOL)containsBrush:(id <Brush>)theBrush;
- (BOOL)intersectsEntity:(id <Entity>)theEntity;
- (BOOL)containsEntity:(id <Entity>)theEntity;

@end

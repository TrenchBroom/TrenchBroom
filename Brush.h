//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Entity;
@class BoundingBox;
@class Vector3f;
@class PickingHit;
@class Ray3D;
@class PickingHitList;

@protocol Brush <NSObject>

- (NSNumber* )brushId;
- (id <Entity>)entity;

- (NSArray *)faces;
- (NSArray *)vertices;
- (NSArray *)edges;

- (float *)flatColor;
- (BoundingBox *)bounds;
- (Vector3f *)center;

- (void)pickBrush:(Ray3D *)theRay hitList:(PickingHitList *)theHitList;
- (void)pickFace:(Ray3D *)theRay hitList:(PickingHitList *)theHitList;
- (void)pickEdge:(Ray3D *)theRay hitList:(PickingHitList *)theHitList;
- (void)pickVertex:(Ray3D *)theRay hitList:(PickingHitList *)theHitList;
- (BoundingBox *)pickingBounds;

@end

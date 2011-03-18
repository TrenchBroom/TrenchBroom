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

@protocol Brush <NSObject>

- (NSNumber* )brushId;
- (id <Entity>)entity;

- (NSArray *)faces;
- (NSArray *)vertices;

- (float *)flatColor;
- (BoundingBox *)bounds;
- (BoundingBox *)pickingBounds;
- (Vector3f *)center;

- (void)pickBrush:(Ray3D *)theRay hits:(NSMutableSet *)theHits;
- (void)pickFace:(Ray3D *)theRay hits:(NSMutableSet *)theHits;
- (void)pickEdge:(Ray3D *)theRay hits:(NSMutableSet *)theHits;
- (void)pickVertex:(Ray3D *)theRay hits:(NSMutableSet *)theHits;

@end

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
- (Vector3f *)center;
- (PickingHit *)pickFace:(Ray3D *)theRay;

@end

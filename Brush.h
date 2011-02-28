//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Entity.h"
#import "BoundingBox.h"
#import "Vector3f.h"
#import "PickingHit.h"
#import "Ray3D.h"

@protocol Brush <NSObject>

- (NSNumber* )brushId;
- (id <Entity>)entity;

- (NSArray *)faces;

- (float *)flatColor;
- (BoundingBox *)bounds;
- (Vector3f *)center;
- (PickingHit *)pickFace:(Ray3D *)theRay;

@end

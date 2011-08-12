//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "VertexData2.h"

@class PickingHit;
@class PickingHitList;
@protocol Entity;
@protocol Face;

@protocol Brush <NSObject, NSCopying>

- (NSNumber* )brushId;
- (id <Entity>)entity;
- (id)copy;

- (NSArray *)faces;
- (const TVertex *)vertices;
- (int)vertexCount;
- (const TEdge *)edges;
- (int)edgeCount;

- (const TBoundingBox *)bounds;
- (const TVector3f *)center;

- (void)pick:(TRay *)theRay hitList:(PickingHitList *)theHitList;
- (void)pickFace:(TRay *)theRay maxDistance:(float)theMaxDist hitList:(PickingHitList *)theHitList;

- (BOOL)intersectsBrush:(id <Brush>)theBrush;
- (BOOL)containsBrush:(id <Brush>)theBrush;
- (BOOL)intersectsEntity:(id <Entity>)theEntity;
- (BOOL)containsEntity:(id <Entity>)theEntity;

@end

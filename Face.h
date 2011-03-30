//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Brush;
@class Vertex;
@class Vector3i;
@class Vector3f;
@class Vector2f;
@class HalfSpace3D;
@class Ray3D;
@class PickingHit;
@class VBOMemBlock;

@protocol Face <NSObject>

- (NSNumber *)faceId;
- (id <Brush>)brush;

- (Vector3i *)point1;
- (Vector3i *)point2;
- (Vector3i *)point3;

- (NSString *)texture;
- (int)xOffset;
- (int)yOffset;
- (float)rotation;
- (float)xScale;
- (float)yScale;

- (Vector3f *)norm;
- (Vector3f *)center;
- (NSArray *)vertices;
- (NSArray *)edges;

- (void)texCoords:(Vector2f *)texCoords forVertex:(Vector3f *)vertex;
- (void)gridCoords:(Vector2f *)gridCoords forVertex:(Vector3f *)vertex;
- (void)transformToWorld:(Vector3f *)point;
- (void)transformToSurface:(Vector3f *)point;
- (HalfSpace3D *)halfSpace;

- (void)setMemBlock:(VBOMemBlock *)theBlock;
- (VBOMemBlock *)memBlock;

@end

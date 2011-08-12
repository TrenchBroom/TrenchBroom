//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

@protocol Brush;
@class Vertex;
@class Matrix4f;
@class PickingHit;
@class VBOMemBlock;

@protocol Face <NSObject, NSCopying>

- (NSNumber *)faceId;
- (id <Brush>)brush;
- (id)copy;

- (TVector3i *)point1;
- (TVector3i *)point2;
- (TVector3i *)point3;

- (NSString *)texture;
- (int)xOffset;
- (int)yOffset;
- (float)rotation;
- (float)xScale;
- (float)yScale;

- (TVector3f *)norm;
- (TVector3f *)center;
- (TPlane *)boundary;
- (NSArray *)vertices;
- (NSArray *)edges;

- (void)texCoords:(TVector2f *)texCoords forVertex:(TVector3f *)vertex;
- (void)gridCoords:(TVector2f *)gridCoords forVertex:(TVector3f *)vertex;
- (void)transformToWorld:(TVector3f *)point;
- (void)transformToSurface:(TVector3f *)point;
- (Matrix4f *)surfaceToWorldMatrix;
- (Matrix4f *)worldToSurfaceMatrix;


- (VBOMemBlock *)memBlock;
- (void)setMemBlock:(VBOMemBlock *)theMemBlock;

@end

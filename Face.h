//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "VertexData.h"

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

- (const TVector3f *)norm;
- (const TVector3f *)center;
- (const TPlane *)boundary;
- (TVertex **)vertices;
- (int)vertexCount;
- (TEdge **)edges;
- (int)edgeCount;

- (void)texCoords:(TVector2f *)texCoords forVertex:(TVector3f *)vertex;
- (void)gridCoords:(TVector2f *)gridCoords forVertex:(TVector3f *)vertex;
- (void)transformSurface:(const TVector3f *)surfacePoint toWorld:(TVector3f *)worldPoint;
- (void)transformWorld:(const TVector3f *)worldPoint toSurface:(TVector3f *)surfacePoint;
- (Matrix4f *)surfaceToWorldMatrix;
- (Matrix4f *)worldToSurfaceMatrix;

- (VBOMemBlock *)memBlock;
- (void)setMemBlock:(VBOMemBlock *)theMemBlock;

@end

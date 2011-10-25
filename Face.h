/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "VertexData.h"

@protocol Brush;
@class Vertex;
@class PickingHit;
@class VBOMemBlock;
@class Texture;

@protocol Face <NSObject, NSCopying>

- (NSNumber *)faceId;
- (id <Brush>)brush;
- (id)copy;

- (TVector3i *)point1;
- (TVector3i *)point2;
- (TVector3i *)point3;

- (Texture *)texture;
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

- (const TBoundingBox *)worldBounds;

- (void)texCoords:(TVector2f *)texCoords forVertex:(TVector3f *)vertex;
- (void)gridCoords:(TVector2f *)gridCoords forVertex:(TVector3f *)vertex;
- (void)transformSurface:(const TVector3f *)surfacePoint toWorld:(TVector3f *)worldPoint;
- (void)transformWorld:(const TVector3f *)worldPoint toSurface:(TVector3f *)surfacePoint;
- (const TMatrix4f *)surfaceToWorldMatrix;
- (const TMatrix4f *)worldToSurfaceMatrix;
- (BOOL)projectToSurface:(const TVector3f *)worldPoint axis:(const TVector3f *)axis result:(TVector3f *)result;

- (VBOMemBlock *)memBlock;
- (void)setMemBlock:(VBOMemBlock *)theMemBlock;

@end

/*
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

@class PickingHit;
@class PickingHitList;
@protocol Entity;
@protocol Face;

@protocol Brush <NSObject, NSCopying>

- (NSNumber* )brushId;
- (id <Entity>)entity;
- (id)copy;

- (NSArray *)faces;
- (TVertex **)vertices;
- (int)vertexCount;
- (TEdge **)edges;
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

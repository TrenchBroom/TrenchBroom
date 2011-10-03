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
#import "Brush.h"
#import "Math.h"
#import "VertexData.h"

@class MutableEntity;
@class MutableFace;
@class Face;
@class PickingHit;

@interface MutableBrush : NSObject <Brush> {
    @private
    NSNumber* brushId;
    MutableEntity* entity;
	NSMutableArray* faces;
    TVertexData vertexData;
    BOOL vertexDataValid;
    int filePosition;
    const TBoundingBox* worldBounds;
}

- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds;
- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds brushTemplate:(id <Brush>)theTemplate;
- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds brushBounds:(const TBoundingBox *)theBrushBounds texture:(NSString *)theTexture;

- (BOOL)addFace:(MutableFace *)face;
- (void)removeFace:(MutableFace *)face;

- (void)setEntity:(MutableEntity *)theEntity;
- (void)translateBy:(const TVector3i *)theDelta lockTextures:(BOOL)lockTextures;
- (void)rotate90CW:(EAxis)theAxis center:(const TVector3i *)theCenter lockTextures:(BOOL)lockTextures;
- (void)rotate90CCW:(EAxis)theAxis center:(const TVector3i *)theCenter lockTextures:(BOOL)lockTextures;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter lockTextures:(BOOL)lockTextures;
- (void)flipAxis:(EAxis)theAxis center:(const TVector3i *)theCenter lockTextures:(BOOL)lockTextures;

- (void)drag:(MutableFace *)face by:(float)dist lockTexture:(BOOL)lockTexture;
- (BOOL)canDrag:(MutableFace *)face by:(float)dist;

- (void)invalidateVertexData;

- (int)filePosition;
- (void)setFilePosition:(int)theFilePosition;
@end

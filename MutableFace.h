/*
Copyright (C) 2010-2012 Kristian Duske

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
#import "Face.h"
#import "Math.h"
#import "VertexData.h"

typedef enum {
    XY, XZ, YZ
} EPlaneType;

@class MutableBrush;
@class PickingHit;
@class VBOMemBlock;
@class Texture;

@interface MutableFace : NSObject <Face> {
    @private
    MutableBrush* brush;
    NSNumber* faceId;
    
	TVector3i point1;
	TVector3i point2;
	TVector3i point3;

	Texture* texture;
	float xOffset;
	float yOffset;
	float rotation;
	float xScale;
	float yScale;
    
    TPlane boundary;
    BOOL boundaryValid;

    TSide* side;
  
    const TVector3f* texPlaneNorm;
    TVector3f texAxisX;
    TVector3f texAxisY;
    TVector3f scaledTexAxisX;
    TVector3f scaledTexAxisY;
    BOOL texAxesValid;
    
    const TBoundingBox* worldBounds;
    
    // transforms surface coordinates to world coordinates
    TMatrix4f surfaceToWorldMatrix;
    TMatrix4f worldToSurfaceMatrix; // inverse of surface matrix
    BOOL matricesValid;
    
    int filePosition;
    
    VBOMemBlock* memBlock;
}

- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds;
- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds point1:(const TVector3i *)aPoint1 point2:(const TVector3i *)aPoint2 point3:(const TVector3i *)aPoint3 texture:(Texture *)theTexture;
- (id)initWithWorldBounds:(const TBoundingBox *)theWorldBounds faceTemplate:(id <Face>)theTemplate;

- (void)setBrush:(MutableBrush *)theBrush;
- (void)setPoint1:(const TVector3i *)thePoint1 point2:(const TVector3i *)thePoint2 point3:(const TVector3i *)thePoint3;
- (void)setTexture:(Texture *)theTexture;
- (void)setXOffset:(int)offset;
- (void)setYOffset:(int)offset;
- (void)setRotation:(float)angle;
- (void)setXScale:(float)factor;
- (void)setYScale:(float)factor;
- (void)translateOffsetsX:(int)x y:(int)y;
- (void)translateBy:(const TVector3i *)theDelta lockTexture:(BOOL)lockTexture;
- (void)rotate90CW:(EAxis)theAxis center:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture;
- (void)rotate90CCW:(EAxis)theAxis center:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter lockTexture:(BOOL)lockTexture;
- (void)flipAxis:(EAxis)theAxis center:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture;
- (void)dragBy:(float)dist lockTexture:(BOOL)lockTexture;

- (void)setSide:(TSide *)theSide;
- (const TSide *)side;

- (int)filePosition;
- (void)setFilePosition:(int)theFilePosition;

- (void)restore:(id <Face>)theTemplate;

@end

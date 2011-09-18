//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Face.h"
#import "Math.h"
#import "VertexData.h"

typedef enum {
    XY, XZ, YZ
} EPlaneType;

@class MutableBrush;
@class Matrix4f;
@class PickingHit;
@class VBOMemBlock;

@interface MutableFace : NSObject <Face> {
    @private
    MutableBrush* brush;
    NSNumber* faceId;
    
	TVector3i point1;
	TVector3i point2;
	TVector3i point3;
	
	NSMutableString* texture;
	float xOffset;
	float yOffset;
	float rotation;
	float xScale;
	float yScale;
    
    TPlane boundary;
    BOOL boundaryValid;
    TVector3f center;
    BOOL centerValid;

    TSide* side;
  
    const TVector3f* texPlaneNorm;
    TVector3f texAxisX;
    TVector3f texAxisY;
    TVector3f scaledTexAxisX;
    TVector3f scaledTexAxisY;
    BOOL texAxesValid;
    
    // transforms surface coordinates to world coordinates
    Matrix4f* surfaceToWorldMatrix;
    Matrix4f* worldToSurfaceMatrix; // inverse of surface matrix
    
    int filePosition;
    
    VBOMemBlock* memBlock;
}

- (id)initWithPoint1:(const TVector3i *)aPoint1 point2:(const TVector3i *)aPoint2 point3:(const TVector3i *)aPoint3 texture:(NSString *)aTexture;
- (id)initWithFaceTemplate:(id <Face>)theTemplate;

- (void)setBrush:(MutableBrush *)theBrush;
- (void)setPoint1:(const TVector3i *)thePoint1 point2:(const TVector3i *)thePoint2 point3:(const TVector3i *)thePoint3;
- (void)setTexture:(NSString *)name;
- (void)setXOffset:(int)offset;
- (void)setYOffset:(int)offset;
- (void)setRotation:(float)angle;
- (void)setXScale:(float)factor;
- (void)setYScale:(float)factor;
- (void)translateOffsetsX:(int)x y:(int)y;
- (void)translateBy:(const TVector3i *)theDelta lockTexture:(BOOL)lockTexture;
- (void)rotateZ90CW:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture;
- (void)rotateZ90CCW:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter lockTexture:(BOOL)lockTexture;
- (void)mirrorAxis:(EAxis)theAxis center:(const TVector3i *)theCenter lockTexture:(BOOL)lockTexture;
- (void)dragBy:(float)dist lockTexture:(BOOL)lockTexture;

- (void)setSide:(TSide *)theSide;

- (int)filePosition;
- (void)setFilePosition:(int)theFilePosition;
@end

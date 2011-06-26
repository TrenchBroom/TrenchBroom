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
	int xOffset;
	int yOffset;
	float rotation;
	float xScale;
	float yScale;
    
    TPlane boundary;
    BOOL boundaryValid;
    TVector3f center;
    BOOL centerValid;

    NSArray* vertices;
    NSArray* edges;
    
    int bestAxis;
    TVector3f texAxisX;
    TVector3f texAxisY;
    BOOL texAxesValid;
    
    // transforms surface coordinates to world coordinates
    Matrix4f* surfaceToWorldMatrix;
    Matrix4f* worldToSurfaceMatrix; // inverse of surface matrix
    
    VBOMemBlock* memBlock;
    
    int filePosition;
}

- (id)initWithPoint1:(TVector3i *)aPoint1 point2:(TVector3i *)aPoint2 point3:(TVector3i *)aPoint3 texture:(NSString *)aTexture;
- (id)initWithFaceTemplate:(id <Face>)theTemplate;

- (void)setBrush:(MutableBrush *)theBrush;
- (void)setPoint1:(TVector3i *)thePoint1 point2:(TVector3i *)thePoint2 point3:(TVector3i *)thePoint3;
- (void)setTexture:(NSString *)name;
- (void)setXOffset:(int)offset;
- (void)setYOffset:(int)offset;
- (void)setRotation:(float)angle;
- (void)setXScale:(float)factor;
- (void)setYScale:(float)factor;
- (void)translateOffsetsX:(int)x y:(int)y;
- (void)translateBy:(TVector3i *)theDelta;
- (void)rotateZ90CW:(TVector3i *)theCenter;
- (void)rotateZ90CCW:(TVector3i *)theCenter;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter;
- (BOOL)canDragBy:(float)dist;
- (void)dragBy:(float)dist;

- (void)setVertices:(NSArray *)theVertices;
- (void)setEdges:(NSArray *)theEdges;

- (int)filePosition;
- (void)setFilePosition:(int)theFilePosition;
@end

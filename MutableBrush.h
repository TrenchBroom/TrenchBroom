//
//  Brush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

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
    TBoundingBox* worldBounds;
}

- (id)initWithWorldBounds:(TBoundingBox *)theWorldBounds;
- (id)initWithWorldBounds:(TBoundingBox *)theWorldBounds brushTemplate:(id <Brush>)theTemplate;
- (id)initWithWorldBounds:(TBoundingBox *)theWorldBounds brushBounds:(TBoundingBox *)theBrushBounds texture:(NSString *)theTexture;

- (BOOL)addFace:(MutableFace *)face;
- (void)removeFace:(MutableFace *)face;

- (void)setEntity:(MutableEntity *)theEntity;
- (void)translateBy:(TVector3i *)theDelta;
- (void)rotateZ90CW:(TVector3i *)theCenter;
- (void)rotateZ90CCW:(TVector3i *)theCenter;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theCenter;
- (void)faceGeometryChanged:(MutableFace *)face;

- (BOOL)canDrag:(MutableFace *)face by:(float)dist;

- (int)filePosition;
- (void)setFilePosition:(int)theFilePosition;
@end

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

@class MutableEntity;
@class MutableFace;
@class Face;
@class VertexData;
@class PickingHit;

@interface MutableBrush : NSObject <Brush> {
    @private
    NSNumber* brushId;
    MutableEntity* entity;
	NSMutableArray* faces;
    VertexData* vertexData;
    int filePosition;
}

- (id)initWithBrushTemplate:(id <Brush>)theTemplate;
- (id)initWithBounds:(TBoundingBox *)theBounds texture:(NSString *)theTexture;

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

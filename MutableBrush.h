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

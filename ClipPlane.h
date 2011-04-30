//
//  ClipPlane.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"

typedef enum {
    CM_FRONT,
    CM_BACK,
    CM_SPLIT
} EClipMode;

@class PickingHitList;
@class MutableFace;
@protocol Face;
@protocol Brush;

@interface ClipPlane : NSObject {
    TVector3i points[3];
    PickingHitList* hitLists[3];
    int numPoints;
    EClipMode clipMode;
}

- (void)addPoint:(TVector3i *)thePoint hitList:(PickingHitList *)theHitList;
- (void)updatePoint:(int)index x:(int)x y:(int)y z:(int)z;
- (void)removeLastPoint;
- (int)numPoints;
- (TVector3i *)point:(int)index;
- (PickingHitList *)hitList:(int)index;

- (void)setClipMode:(EClipMode)theClipMode;
- (EClipMode)clipMode;

- (MutableFace *)face:(BOOL)front;
- (void)clipBrush:(id <Brush>)brush firstResult:(id <Brush>*)firstResult secondResult:(id <Brush>*)secondResult;
- (void)reset;

@end

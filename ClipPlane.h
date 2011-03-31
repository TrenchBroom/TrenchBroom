//
//  ClipPlane.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    CM_FRONT,
    CM_BACK,
    CM_SPLIT
} EClipMode;

@class PickingHitList;
@class Vector3i;
@class MutableFace;
@protocol Face;
@protocol Brush;

@interface ClipPlane : NSObject {
    Vector3i* point1;
    Vector3i* point2;
    Vector3i* point3;
    PickingHitList* hitList1;
    PickingHitList* hitList2;
    PickingHitList* hitList3;
    EClipMode clipMode;
}

- (void)setPoint1:(Vector3i *)thePoint1;
- (void)setPoint2:(Vector3i *)thePoint2;
- (void)setPoint3:(Vector3i *)thePoint3;
- (void)setHitList1:(PickingHitList *)theHitList1;
- (void)setHitList2:(PickingHitList *)theHitList2;
- (void)setHitList3:(PickingHitList *)theHitList3;
- (void)setClipMode:(EClipMode)theClipMode;

- (Vector3i *)point1;
- (Vector3i *)point2;
- (Vector3i *)point3;
- (EClipMode)clipMode;
- (MutableFace *)face:(BOOL)front;

- (void)clipBrush:(id <Brush>)brush firstResult:(id <Brush>*)firstResult secondResult:(id <Brush>*)secondResult;

@end

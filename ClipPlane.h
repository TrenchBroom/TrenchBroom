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

@class Vector3i;
@class MutableFace;
@protocol Face;

@interface ClipPlane : NSObject {
    Vector3i* point1;
    Vector3i* point2;
    Vector3i* point3;
    id <Face> face1;
    id <Face> face2;
    id <Face> face3;
    EClipMode clipMode;
}

- (void)setPoint1:(Vector3i *)thePoint1;
- (void)setPoint2:(Vector3i *)thePoint2;
- (void)setPoint3:(Vector3i *)thePoint3;
- (void)setFace1:(id <Face>)theFace1;
- (void)setFace2:(id <Face>)theFace2;
- (void)setFace3:(id <Face>)theFace3;
- (void)setClipMode:(EClipMode)theClipMode;

- (Vector3i *)point1;
- (Vector3i *)point2;
- (Vector3i *)point3;
- (EClipMode)clipMode;
- (MutableFace *)frontFace;
- (MutableFace *)backFace;

@end

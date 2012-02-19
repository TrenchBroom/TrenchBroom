//
//  DragTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.02.12.
//  Copyright (c) 2012 TU Berlin. All rights reserved.
//

#import "DefaultTool.h"
#import "Math.h"

@class DragPlane;

@interface DragTool : DefaultTool {
    @private
    DragPlane* dragPlane;
    TVector3f dragPlanePosition;
    TVector3f lastPoint;
    BOOL drag;
}

- (BOOL)doBeginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint;
- (BOOL)doLeftDrag:(NSEvent *)event ray:(TRay *)ray delta:(TVector3f *)delta hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint;
- (void)doEndLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

- (BOOL)doBeginRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint;
- (BOOL)doRightDrag:(NSEvent *)event ray:(TRay *)ray delta:(TVector3f *)delta hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint;
- (void)doEndRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

@end

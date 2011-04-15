//
//  Tool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 27.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PickingHitList;
@class Ray3D;

@protocol Tool <NSObject>

- (void)handleFlagsChanged:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (BOOL)handleLeftMouseDown:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)handleLeftMouseUp:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)handleRightMouseDown:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)handleRightMouseUp:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)handleMouseMoved:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)handleScrollWheel:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (BOOL)handleBeginGesture:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)handleEndGesture:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)handleMagnify:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (BOOL)beginLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)leftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)endLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (BOOL)beginRightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)rightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (BOOL)endRightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (BOOL)hasCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)setCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)unsetCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)updateCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (NSString *)actionName;
@end

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

- (void)handleLeftMouseDown:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)handleLeftMouseUp:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)handleRightMouseDown:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)handleRightMouseUp:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)handleMouseMoved:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)handleScrollWheel:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (void)handleBeginGesture:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)handleEndGesture:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)handleMagnify:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (void)beginLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)leftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)endLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (void)beginRightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)rightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;
- (void)endRightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (void)updateCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits;

- (NSString *)actionName;
@end

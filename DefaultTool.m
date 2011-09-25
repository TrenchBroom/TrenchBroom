//
//  DefaultTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultTool.h"

@implementation DefaultTool

- (void)activated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)deactivated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)handleFlagsChanged:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)handleLeftMouseDown:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)handleLeftMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)handleRightMouseDown:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)handleRightMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)handleMouseMoved:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)handleScrollWheel:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)handleBeginGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)handleEndGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)handleMagnify:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)beginRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)rightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)endRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)beginLeftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)leftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)endLeftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)beginRightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)rightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)endRightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)setCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)unsetCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)updateCursor:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (NSString *)actionName { return nil; }

@end

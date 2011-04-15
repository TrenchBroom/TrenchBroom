//
//  DefaultTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 14.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultTool.h"


@implementation DefaultTool

- (void)handleFlagsChanged:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {}

- (BOOL)handleLeftMouseDown:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)handleLeftMouseUp:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)handleRightMouseDown:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)handleRightMouseUp:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)handleMouseMoved:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)handleScrollWheel:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }

- (BOOL)handleBeginGesture:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)handleEndGesture:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)handleMagnify:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }

- (BOOL)beginLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)leftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)endLeftDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }

- (BOOL)beginRightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)rightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)endRightDrag:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }

- (BOOL)hasCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits { return NO; }
- (void)setCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {}
- (void)unsetCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {}
- (void)updateCursor:(NSEvent *)event ray:(Ray3D *)ray hits:(PickingHitList *)hits {}

- (NSString *)actionName { return nil; }

@end

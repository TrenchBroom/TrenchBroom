/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "DefaultTool.h"

@implementation DefaultTool

- (void)activated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)deactivated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)handleKeyStatusChanged:(NSEvent *)event status:(EKeyStatus)keyStatus ray:(TRay *)ray hits:(PickingHitList *)hits {}

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

- (NSString *)actionName { return nil; }

@end

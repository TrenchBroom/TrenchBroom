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

#import <Cocoa/Cocoa.h>
#import "Math.h"

@class PickingHitList;

@protocol Tool <NSObject>

- (void)activated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)deactivated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

- (void)leftMouseDown:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)leftMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)rightMouseDown:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)rightMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)mouseMoved:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)scrollWheel:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

- (void)beginGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)endGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)magnify:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

- (void)beginRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)rightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)endRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

- (void)beginLeftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)leftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)endLeftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

- (void)beginRightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)rightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)endRightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits;

- (NSString *)actionName;
@end

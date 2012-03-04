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

#import "Tool.h"
#import "MapWindowController.h"
#import "Figure.h"
#import "Renderer.h"

@implementation Tool

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
    }
    
    return self;
}

- (void)addFeedbackFigure:(id <Figure>)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    
    Renderer* renderer = [windowController renderer];
    [renderer addFeedbackFigure:theFigure];
}

- (void)removeFeedbackFigure:(id <Figure>)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    
    Renderer* renderer = [windowController renderer];
    [renderer removeFeedbackFigure:theFigure];
}

- (void)updateFeedbackFigure:(id <Figure>)theFigure  {
    NSAssert(theFigure != nil, @"figure must not be nil");
    
    Renderer* renderer = [windowController renderer];
    [renderer updateFeedbackFigure:theFigure];
}

- (void)activated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)deactivated:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (BOOL)leftMouseDown:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)leftMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)rightMouseDown:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (BOOL)rightMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (void)mouseMoved:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (BOOL)scrollWheel:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }

- (BOOL)beginGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (void)endGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)magnify:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (BOOL)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (BOOL)beginRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (void)rightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)endRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (BOOL)beginLeftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (void)leftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)endLeftScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (BOOL)beginRightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits { return NO; }
- (void)rightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}
- (void)endRightScroll:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (void)flagsChanged:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (NSString *)actionName { return nil; }

@end

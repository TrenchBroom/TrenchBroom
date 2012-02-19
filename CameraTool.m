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

#import "CameraTool.h"
#import "MapWindowController.h"
#import "Camera.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@interface CameraTool (private)

- (BOOL)isCameraModifierPressed;
- (BOOL)isCameraOrbitModifierPressed;

@end

@implementation CameraTool (private)

- (BOOL)isCameraModifierPressed {
    return [NSEvent modifierFlags] == NSShiftKeyMask;
}

- (BOOL)isCameraOrbitModifierPressed {
    return [NSEvent modifierFlags] == (NSShiftKeyMask | NSCommandKeyMask);
}

@end

@implementation CameraTool

- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isCameraModifierPressed] && ![self isCameraOrbitModifierPressed])
        return;
    
    if ([self isCameraOrbitModifierPressed]) {
        PickingHit* hit = [hits firstHitOfType:HT_ANY ignoreOccluders:YES];
        if (hit != nil) {
            orbitCenter = *[hit hitPoint];
        } else {
            Camera* camera = [windowController camera];
            orbitCenter = [camera defaultPoint];
        }
        orbit = YES;
    }
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    orbit = NO;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    Camera* camera = [windowController camera];
    if (orbit) {
        float h = -[event deltaX] / 90;
        float v = [event deltaY] / 90;
        [camera orbitCenter:&orbitCenter hAngle:h vAngle:v];
    } else {
        float yaw = -[event deltaX] / 90;
        float pitch = [event deltaY] / 90;
        [camera rotateYaw:yaw pitch:pitch];
    }
}

- (void)rightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isCameraModifierPressed])
        return;
    
    Camera* camera = [windowController camera];
    [camera moveForward:0 right:[event deltaX] up:-[event deltaY]];
}

- (void)handleBeginGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    gesture = YES;
}

- (void)handleEndGesture:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    gesture = NO;
}

- (void)handleScrollWheel:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isCameraModifierPressed] && ![self isCameraOrbitModifierPressed])
        return;
    
    Camera* camera = [windowController camera];
    if ([self isCameraOrbitModifierPressed]) {
        if (gesture)
            [camera setZoom:[camera zoom] + [event deltaZ] / 10];
        else
            [camera setZoom:[camera zoom] + [event deltaX] / 10];
    } else {
        if (gesture)
            [camera moveForward:6 * [event deltaZ] right:-6 * [event deltaX] up:6 * [event deltaY]];
        else
            [camera moveForward:6 * [event deltaX] right:-6 * [event deltaY] up:6 * [event deltaZ]];
    }
}

- (void)handleMagnify:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isCameraModifierPressed])
        return;
    
    Camera* camera = [windowController camera];
    if ([self isCameraOrbitModifierPressed])
        [camera setZoom:[camera zoom] - [event magnification] / 2];
    else
        [camera moveForward:160 * [event magnification] right:0 up:0];
}

@end

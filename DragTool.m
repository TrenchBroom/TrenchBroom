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

#import "DragTool.h"
#import "MapWindowController.h"
#import "Camera.h"
#import "DragPlane.h"

@interface DragTool (private)

- (BOOL)isAlternatePlaneModifierPressed;
- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits;

@end

@implementation DragTool (private)

- (BOOL)isAlternatePlaneModifierPressed {
    return ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0;
}

- (void)updateMoveDirectionWithRay:(const TRay *)theRay hits:(PickingHitList *)theHits {
    if (dragPlane != nil)
        [dragPlane release];
    
    Camera* camera = [windowController camera];
    dragPlane = [[DragPlane alloc] initWithCamera:camera vertical:[self isAlternatePlaneModifierPressed]];
}

@end

@implementation DragTool

- (BOOL)doBeginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint { return NO; }
- (BOOL)doLeftDrag:(NSEvent *)event ray:(TRay *)ray delta:(TVector3f *)delta hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint { return NO; }
- (void)doEndLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}

- (BOOL)doBeginRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint { return NO; }
- (BOOL)doRightDrag:(NSEvent *)event ray:(TRay *)ray delta:(TVector3f *)delta hits:(PickingHitList *)hits lastPoint:(TVector3f *)lastPoint { return NO; }

- (void)doEndRightDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {}


- (void)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    [self updateMoveDirectionWithRay:ray hits:hits];
    drag = [self doBeginLeftDrag:event ray:ray hits:hits lastPoint:&lastPoint];
    dragPlanePosition = lastPoint;
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    float dist = [dragPlane intersectWithRay:ray planePosition:&dragPlanePosition];
    if (isnan(dist))
        return;
    
    TVector3f point;
    rayPointAtDistance(ray, dist, &point);
    
    TVector3f delta;
    subV3f(&point, &lastPoint, &delta);
    
    if (![self doLeftDrag:event ray:ray delta:&delta hits:hits lastPoint:&lastPoint])
        [self endLeftDrag:event ray:ray hits:hits];
}

- (void)endLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (!drag)
        return;
    
    [self doEndLeftDrag:event ray:ray hits:hits];

    if (dragPlane != nil) {
        [dragPlane release];
        dragPlane = nil;
    }
    
    drag = NO;
}

- (void)dealloc {
    if (dragPlane != nil)
        [dragPlane release];
    [super dealloc];
}

@end

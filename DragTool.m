//
//  DragTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.02.12.
//  Copyright (c) 2012 TU Berlin. All rights reserved.
//

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

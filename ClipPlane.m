//
//  ClipPlane.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipPlane.h"
#import "Vector3i.h"
#import "Vector3f.h"
#import "Face.h"
#import "MutableFace.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "PickingHit.h"
#import "PickingHitList.h"

@implementation ClipPlane

- (void)setPoint1:(Vector3i *)thePoint1 {
    [point1 release];
    point1 = [thePoint1 retain];
}

- (void)setPoint2:(Vector3i *)thePoint2 {
    [point2 release];
    point2 = [thePoint2 retain];
}

- (void)setPoint3:(Vector3i *)thePoint3 {
    [point3 release];
    point3 = [thePoint3 retain];
}

- (void)setHitList1:(PickingHitList *)theHitList1 {
    [hitList1 release];
    hitList1 = [theHitList1 retain];
}

- (void)setHitList2:(PickingHitList *)theHitList2 {
    [hitList2 release];
    hitList2 = [theHitList2 retain];
}

- (void)setHitList3:(PickingHitList *)theHitList3 {
    [hitList3 release];
    hitList3 = [theHitList3 retain];
}


- (void)setClipMode:(EClipMode)theClipMode {
    clipMode = theClipMode;
}


- (Vector3i *)point1 {
    return point1;
}

- (Vector3i *)point2 {
    return point2;
}

- (Vector3i *)point3 {
    return point3;
}

- (EClipMode)clipMode {
    return clipMode;
}

- (MutableFace *)face:(BOOL)front {
    if (point2 == nil)
        return nil;

    Vector3i* p3 = point3;
    if (p3 == nil) {
        Vector3f* norm = nil;
        if ([point1 x] != [point2 x] &&
            [point1 y] != [point2 y] &&
            [point1 z] != [point2 z]) {
            norm = [[Vector3f alloc] initWithFloatVector:[Vector3f zAxisPos]];
        } else if ([point1 x] == [point2 x] &&
                   [point1 y] != [point2 y] &&
                   [point1 z] != [point2 z]) {
            norm = [[Vector3f alloc] initWithFloatVector:[Vector3f xAxisPos]];
        } else if ([point1 x] != [point2 x] &&
                   [point1 y] == [point2 y] &&
                   [point1 z] != [point2 z]) {
            norm = [[Vector3f alloc] initWithFloatVector:[Vector3f yAxisPos]];
        } else if ([point1 x] != [point2 x] &&
                   [point1 y] != [point2 y] &&
                   [point1 z] == [point2 z]) {
            norm = [[Vector3f alloc] initWithFloatVector:[Vector3f zAxisPos]];
        } else {
            NSSet* faces1 = [hitList1 objectsOfType:HT_FACE];
            NSSet* faces2 = [hitList2 objectsOfType:HT_FACE];
            
            NSSet* both = [[NSMutableSet alloc] initWithSet:faces1];
            [both intersectsSet:faces2];
            
            if ([both count] > 0) {
                id <Face> face = [[both objectEnumerator] nextObject];
                norm = [[Vector3f alloc] initWithFloatVector:[face norm]];
            }
            
            [both release];
        }
        
        if (norm != nil) {
            [norm scale:10000];
            
            p3 = [[Vector3i alloc] init];
            [p3 setX:roundf([norm x])];
            [p3 setY:roundf([norm y])];
            [p3 setZ:roundf([norm z])];
            [p3 add:point1];
            
            [norm release];
        }
    }
    
    
    if (p3 == nil)
        return nil;
    
    id <Face> template = [[hitList1 firstHitOfType:HT_FACE ignoreOccluders:YES] object];
    
    MutableFace* face = nil;
    if (front)
        face = [[MutableFace alloc] initWithPoint1:point1 point2:point2 point3:p3 texture:[template texture]];
    else
        face = [[MutableFace alloc] initWithPoint1:p3 point2:point2 point3:point1 texture:[template texture]];

    [face setXOffset:[template xOffset]];
    [face setYOffset:[template yOffset]];
    [face setXScale:[template xScale]];
    [face setYScale:[template yScale]];
    [face setRotation:[template rotation]];
    
    if (p3 != point3)
        [p3 release];
    
    return [face autorelease];
}

- (void)clipBrush:(id <Brush>)brush firstResult:(id <Brush>*)firstResult secondResult:(id <Brush>*)secondResult {
    MutableBrush* newBrush = [[MutableBrush alloc] initWithBrushTemplate:brush];
    MutableFace* clipFace = clipMode == CM_BACK ? [self face:NO] : [self face:YES];
    if (clipFace != nil && ![newBrush addFace:clipFace]) {
        [newBrush release];
        newBrush = nil;
    } else {
        *firstResult = newBrush;
        [newBrush autorelease];
    }
    
    if (clipMode == CM_SPLIT) {
        newBrush = [[MutableBrush alloc] initWithBrushTemplate:brush];
        clipFace = [self face:NO];
        if (clipFace != nil && ![newBrush addFace:clipFace]) {
            [newBrush release];
            newBrush = nil;
        } else {
            *secondResult = newBrush;
            [newBrush autorelease];
        }
    }
}

- (void)reset {
    [point1 release];
    point1 = nil;
    [point2 release];
    point2 = nil;
    [point3 release];
    point3 = nil;
    [hitList1 release];
    hitList1 = nil;
    [hitList2 release];
    hitList2 = nil;
    [hitList3 release];
    hitList3 = nil;
}

- (void)dealloc {
    [point1 release];
    [point2 release];
    [point3 release];
    [hitList1 release];
    [hitList2 release];
    [hitList3 release];
    [super dealloc];
}

@end

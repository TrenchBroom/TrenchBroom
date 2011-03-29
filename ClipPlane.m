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

- (void)setFace1:(id <Face>)theFace1 {
    [face1 release];
    face1 = [theFace1 retain];
}

- (void)setFace2:(id <Face>)theFace2 {
    [face2 release];
    face2 = [theFace2 retain];
}

- (void)setFace3:(id <Face>)theFace3 {
    [face3 release];
    face3 = [theFace3 retain];
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
        Vector3f* norm = [[Vector3f alloc] initWithFloatVector:[face1 norm]];
        if (face2 != face1) {
            [norm add:[face2 norm]];
            [norm scale:0.5f];
            
            if ([norm isNull])
                [norm setFloat:[face1 norm]];
        }
        
        [norm scale:10000];
        
        p3 = [[Vector3i alloc] init];
        [p3 setX:roundf([norm x])];
        [p3 setY:roundf([norm y])];
        [p3 setZ:roundf([norm z])];
        
        [norm release];
    }
    
    id <Face> template = face3;
    if (template == nil)
        template = face2;
    if (template == nil || face1 == face2)
        template = face1;
    
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

- (void)dealloc {
    [point1 release];
    [point2 release];
    [point3 release];
    [face1 release];
    [face2 release];
    [face3 release];
    [super dealloc];
}

@end

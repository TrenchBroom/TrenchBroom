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

- (MutableFace *)frontFace {
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
    
    MutableFace* face = [[MutableFace alloc] initWithPoint1:point1 point2:point2 point3:p3 texture:@""];
    
    if (p3 != point3)
        [p3 release];
    
    return face;
}

- (MutableFace *)backFace {
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
    
    MutableFace* face = [[MutableFace alloc] initWithPoint1:p3 point2:point2 point3:point1 texture:@""];
    
    if (p3 != point3)
        [p3 release];
    
    return face;
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

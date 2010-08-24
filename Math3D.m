//
//  Math3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.05.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Math3D.h"

float const AlmostZero = 0.0000001f;

@implementation Math3D

+ (Side)turnDirectionFrom:(Vector3f *)from to:(Vector3f *)to normal:(Vector3f *)normal {
    if (!from)
        [NSException raise:NSInvalidArgumentException format:@"from must not be nil"];
    if (!to)
        [NSException raise:NSInvalidArgumentException format:@"to must not be nil"];
    if (!normal)
        [NSException raise:NSInvalidArgumentException format:@"normal must not be nil"];
    
    Vector3f* c = [Vector3f cross:normal factor:from];
    if ([c isNull])
        [NSException raise:NSInvalidArgumentException format:@"from vector and normal vector must not be colinear"];

    float d = [c dot:to];
    if (d > AlmostZero)
        return SLeft;
    if (d < -AlmostZero)
        return SRight;
    return SNeither;
}

@end

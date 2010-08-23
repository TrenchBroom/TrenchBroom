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

+ (Side)turnDirectionFrom:(Vector3f *)from to:(Vector3f *)to up:(Vector3f *)up {
    if (!from)
        [NSException raise:NSInvalidArgumentException format:@"from must not be nil"];
    if (!to)
        [NSException raise:NSInvalidArgumentException format:@"to must not be nil"];
    if (!up)
        [NSException raise:NSInvalidArgumentException format:@"up must not be nil"];
    
    Vector3f* c = [Vector3f cross:up factor:from];
    if ([c isNull])
        [NSException raise:NSInvalidArgumentException format:@"from vector and up vector must not be colinear"];

    float d = [c dot:to];
    if (d > AlmostZero)
        return SLeft;
    if (d < -AlmostZero)
        return SRight;
    return SNeither;
}

@end

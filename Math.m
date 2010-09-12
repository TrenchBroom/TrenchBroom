//
//  Math.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Math.h"

float const AlmostZero = 0.000001f;

@implementation Math
+ (BOOL)is:(float)v betEx:(float)b1 andEx:(float)b2 {
    if (b1 < b2)
        return [Math is:v gt:b1] && [Math is:v lt:b2];
    return [Math is:v gt:b2] && [Math is:v lt:b1];
}

+ (BOOL)is:(float)v betEx:(float)b1 andIn:(float)b2 {
    if (b1 < b2)
        return [Math is:v gt:b1] && [Math is:v lte:b2];
    return [Math is:v gte:b2] && [Math is:v lt:b1];
}

+ (BOOL)is:(float)v betIn:(float)b1 andEx:(float)b2 {
    if (b1 < b2)
        return [Math is:v gte:b1] && [Math is:v lt:b2];
    return [Math is:v gt:b2] && [Math is:v lte:b1];
}

+ (BOOL)is:(float)v betIn:(float)b1 andIn:(float)b2 {
    if (b1 < b2)
        return [Math is:v gte:b1] && [Math is:v lte:b2];
    return [Math is:v gte:b2] && [Math is:v lte:b1];
}

+ (BOOL)is:(float)v1 eq:(float)v2 {
    return fabsf(v1 - v2) < AlmostZero;
}

+ (BOOL)is:(float)v1 gt:(float)v2 {
    return v1 > v2 + AlmostZero;
}

+ (BOOL)is:(float)v1 lt:(float)v2 {
    return v1 < v2 - AlmostZero;
}

+ (BOOL)is:(float)v1 gte:(float)v2 {
    return [Math is:v1 eq:v2] || [Math is:v1 gt:v2];
}

+ (BOOL)is:(float)v1 lte:(float)v2 {
    return [Math is:v1 eq:v2] || [Math is:v1 lt:v2];
}

+ (BOOL)pos:(float)v {
    return v > AlmostZero;
}

+ (BOOL)neg:(float)v {
    return v < -AlmostZero;
}

+ (BOOL)zer:(float)v {
    return fabsf(v) <= AlmostZero;
}

@end

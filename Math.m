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
+ (BOOL)is:(float)v between:(float)b1 and:(float)b2 {
    if (b1 < b2)
        return v >= b1 && v <= b2;
    return v >= b2 && v <= b1;
}
@end

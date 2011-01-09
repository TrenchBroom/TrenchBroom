//
//  Math2D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Math2D.h"
#import "Math.h"
#import "Vector2f.h"

@implementation Math2D
+ (ESide2D)turnDirectionFrom:(Vector2f *)from to:(Vector2f *)to {
    if (!from)
        [NSException raise:NSInvalidArgumentException format:@"from vector must not be nil"];
    if (!to)
        [NSException raise:NSInvalidArgumentException format:@"to vector must not be nil"];
    
	// rotate from by 90 degs clockwise
	Vector2f* t = [[Vector2f alloc] init];
	[t setX:[from y]];
	[t setY:-[from x]];
    
	float d = [t dot:to];
    if (fpos(d))
		return SRight;
    if (fneg(d))
		return SLeft;
	return SNeither;
}

@end

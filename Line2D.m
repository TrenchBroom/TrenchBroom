//
//  Line2D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 20.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Line2D.h"
#import "Math3D.h"

@implementation Line2D
- (id)initWithPoint:(Vector2f *)point normalizedDirection:(Vector2f *)dir {
	if (!point || !dir) {
		[self release];
		return nil;
	}
	
	if (self = [super init]) {
        p = [[Vector2f alloc] initWithVector:point];
        d = [[Vector2f alloc] initWithVector:dir];
    }
	
	return self;
}

- (id)initWithPoint:(Vector2f *)point direction:(Vector2f *)dir {
	if (!point || !dir) {
		[self release];
		return nil;
	}
	
	if (self = [super init]) {
        p = [[Vector2f alloc] initWithVector:point];
        d = [[Vector2f alloc] initWithVector:dir];
        [d normalize];
    }
	
	return self;
}

- (Vector2f *)p {
    return p;
}

- (Vector2f *)d {
    return d;
}

- (Vector2f *)intersectionWithLine:(Line2D *)line {
    float xp1 = [p x];
    float yp1 = [p y];
    float xd1 = [d x];
    float yd1 = [d y];
    float xp2 = [[line p] x];
    float yp2 = [[line p] y];
    float xd2 = [[line d] x];
    float yd2 = [[line d] y];
    
    float t = (yd2 * (xp2 - xp1) + xd2 * (yp1 - yp2)) / (yd2 * xd1 - yd1 * xd2);

    float x = xp1 + t * xd1;
    float y = yp1 + t * yd1;
    float sx = (x - xp2) / xd2;
    float sy = (y - yp2) / yd2;
    
    if (abs(sx - sy) > AlmostZero)
        return nil;
    
    Vector2f* v = [[Vector2f alloc] initWithXCoord:x yCoord:y];
    return [v autorelease];
}

- (void)dealloc {
    [p release];
    [d release];
    [super dealloc];
}
@end

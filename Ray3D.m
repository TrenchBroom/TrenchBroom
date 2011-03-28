//
//  Ray3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Ray3D.h"
#import "Vector3f.h"

@implementation Ray3D
- (id)initWithOrigin:(Vector3f *)theOrigin direction:(Vector3f *)theDirection {
    if (self = [self init]) {
        origin = [theOrigin retain];
        direction = [theDirection retain];
    }
    
    return self;
}

- (id)initWithOrigin:(Vector3f *)theOrigin point:(Vector3f *)thePoint {
    [thePoint sub:theOrigin];
    [thePoint normalize];
    
    return [self initWithOrigin:theOrigin direction:thePoint];
}

- (Vector3f *)origin {
    return origin;
}

- (Vector3f *)direction {
    return direction;
}

- (Vector3f *)pointAtDistance:(float)distance {
    if (isnan(distance))
        return nil;
    
    Vector3f* point = [[Vector3f alloc] initWithFloatVector:direction];
    [point scale:distance];
    [point add:origin];
    return [point autorelease];
}

- (float)intersectWithSphere:(Vector3f *)center radius:(float)radius {
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:origin];
    [diff sub:center];
    
    float p = 2 * [direction dot:diff];
    float q = [diff lengthSquared] - radius * radius;
    [diff release];
    
    float d = p * p - 4 * q;
    if (d < 0)
        return NAN;
    
    float r = sqrt(d);
    float t0 = -p + r;
    float t1 = -p - r;

    if (t0 < 0 && t1 < 0)
        return NAN;
    if (t0 > 0 && t1 > 0)
        return fmin(t0, t1);
    return fmax(t0, t1);
}

- (void)dealloc {
    [origin release];
    [direction release];
    [super dealloc];
}
@end

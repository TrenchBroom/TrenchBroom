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
    if (theOrigin == nil)
        [NSException raise:NSInvalidArgumentException format:@"origin must not be nil"];
    if (theDirection == nil)
        [NSException raise:NSInvalidArgumentException format:@"direction must not be nil"];

    if (self = [self init]) {
        origin = [theOrigin retain];
        direction = [theDirection retain];
    }
    
    return self;
}

- (id)initWithOrigin:(Vector3f *)theOrigin point:(Vector3f *)thePoint {
    if (theOrigin == nil)
        [NSException raise:NSInvalidArgumentException format:@"origin must not be nil"];
    if (thePoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    
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

- (void)dealloc {
    [origin release];
    [direction release];
    [super dealloc];
}
@end

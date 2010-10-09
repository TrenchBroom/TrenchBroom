//
//  Plane3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Plane3D.h"
#import "Math.h"

@implementation Plane3D
+ (Plane3D *)planeWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm {
    return [[[Plane3D alloc] initWithPoint:aPoint norm:aNorm] autorelease];
}

- (id)initWithPoint:(Vector3f *)aPoint norm:(Vector3f *)aNorm {
    if (aPoint == nil)
        [NSException raise:NSInvalidArgumentException format:@"point must not be nil"];
    if (aNorm == nil)
        [NSException raise:NSInvalidArgumentException format:@"normal must not be nil"];
    
    if (self == [super init]) {
        point = [[Vector3f alloc] initWithFloatVector:aPoint];
        norm = [[Vector3f alloc] initWithFloatVector:aNorm];
    }
    
    return self;
}

- (Vector3f *)intersectWith:(Line3D *)line {
    if (line == nil)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    Vector3f* lp = [line point];
    Vector3f* ld = [line direction];
    float denom = [ld dot:norm];
    if (fzero(denom))
        return nil;

    Vector3f* diff = [Vector3f sub:point subtrahend:lp];
    float d = [diff dot:norm] / denom;
    
    Vector3f* is = [[Vector3f alloc] initWithFloatVector:ld];
    [is scale:d];
    [is add:lp];
    
    return [is autorelease];
}

- (void)dealloc {
    [point release];
    [norm release];
    [super dealloc];
}

@end

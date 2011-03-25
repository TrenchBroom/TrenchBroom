//
//  Quaternion.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Quaternion.h"
#import "Vector3f.h"
#import "math.h"

@implementation Quaternion
+ (Quaternion *)quaternionWithScalar:(float)theScalar vector:(Vector3f *)theVector {
    return [[[self alloc] initWithScalar:theScalar vector:theVector] autorelease];
}

+ (Quaternion *)quaternionWithAngle:(float)theAngle axis:(Vector3f *)theAxis {
    return [[[self alloc] initWithAngle:theAngle axis:theAxis] autorelease];
}

+ (Quaternion *)mul:(Quaternion *)left with:(Quaternion *)right {
    Quaternion* result = [[Quaternion alloc] initWithQuaternion:left];
    [result mul:left];
    return [result autorelease];
}

+ (Quaternion *)conjugate:(Quaternion *)quaternion {
    Quaternion* result = [[Quaternion alloc] initWithQuaternion:quaternion];
    [result conjugate];
    return [result autorelease];
}


- (id)init {
    if (self = [super init]) {
        a = 0;
        v = [[Vector3f alloc] init];
    }
    
    return self;
}

- (id)initWithQuaternion:(Quaternion *)quaternion {
    if (self = [self init]) {
        [self setQuaternion:quaternion];
    }
    
    return self;
}

- (id)initWithScalar:(float)theScalar vector:(Vector3f *)theVector {
    if (self = [self init]) {
        [self setScalar:theScalar];
        [self setVector:theVector];
    }
    
    return self;
}

- (id)initWithAngle:(float)theAngle axis:(Vector3f *)theAxis {
    if (self = [self init]) {
        [self setAngle:theAngle axis:theAxis];
    }
    
    return self;
}

- (float)scalar {
    return a;
}

- (Vector3f *)vector {
    return v;
}

- (void)setScalar:(float)theScalar {
    a = theScalar;
}

- (void)setVector:(Vector3f *)theVector {
    [v setFloat:theVector];
}

- (void)setQuaternion:(Quaternion *)theQuaternion {
    a = [theQuaternion scalar];
    [v setFloat:[theQuaternion vector]];
}

- (void)setAngle:(float)theAngle axis:(Vector3f *)theAxis {
    a = cos(theAngle / 2);
    [v setFloat:theAxis];
    [v scale:sin(theAngle / 2)];
}

- (void)mul:(Quaternion *)right {
    float b = [right scalar];
    Vector3f* w = [right vector];
    
    float nx = a * [w x] + b * [v x] + [v y] * [w z] - [v z] * [w y];
    float ny = a * [w y] + b * [v y] + [v z] * [w x] - [v x] * [w z];
    float nz = a * [w z] + b * [v z] + [v x] * [w y] - [v y] * [w x];
    
    a = a * b - [v dot:w];

    [v setX:nx];
    [v setY:ny];
    [v setZ:nz];
}

- (void)conjugate {
    [v scale:-1];
}

- (void)rotate:(Vector3f *)theVector {
    Quaternion* q = [[Quaternion alloc] initWithQuaternion:self];
    Quaternion* p = [[Quaternion alloc] initWithScalar:0 vector:theVector];
    Quaternion* c = [[Quaternion alloc] initWithQuaternion:self];
    [c conjugate];

    [q mul:p];
    [q mul:c];
    
    [theVector setFloat:[q vector]];
    [p release];
    [q release];
    [c release];
}

- (void)dealloc {
    [v release];
    [super dealloc];
}

@end

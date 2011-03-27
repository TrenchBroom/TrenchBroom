//
//  Vector4f.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Vector4f.h"
#import "Vector3f.h"
#import "Math.h"
#import "math.h"

@implementation Vector4f
- (id)init {
    if (self = [super init]) {
        x = 0;
        y = 0; 
        z = 0;
        w = 0;
    }
    
    return self;
}

- (id)initWithVector3f:(Vector3f *)vector {
    if (self = [super init]) {
        [self setVector3f:vector];
    }
    
    return self;
}

- (id)initWithVector4f:(Vector4f *)vector {
    if (self = [super init]) {
        [self setVector4f:vector];
    }
    
    return self;
}

- (id)initWithFloatX:(float)xCoord y:(float)yCoord z:(float)zCoord w:(float)wCoord {
    if (self = [super init]) {
        x = xCoord;
        y = yCoord;
        z = zCoord;
        w = wCoord;
    }
    
    return self;
}

- (float)x {
    return x;
}

- (float)y {
    return y;
}

- (float)z {
    return z;
}

- (float)w {
    return w;
}

- (float)component:(int)index {
    switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        case 3:
            return w;
    }
    
    [NSException raise:NSInvalidArgumentException format:@"component index out of bounds: %i", index];
    return 0; // cannot happen
}

- (void)setComponent:(int)index value:(float)value {
    switch (index) {
        case 0:
            x = value;
            break;
        case 1:
            y = value;
            break;
        case 2:
            z = value;
            break;
        case 3:
            w = value;
            break;
    }
    
    [NSException raise:NSInvalidArgumentException format:@"component index out of bounds: %i", index];
}

- (void)setX:(float)xCoord {
    x = xCoord;
}

- (void)setY:(float)yCoord {
    y = yCoord;
}

- (void)setZ:(float)zCoord {
    z = zCoord;
}

- (void)setW:(float)wCoord {
    w = wCoord;
}

- (void)setVector3f:(Vector3f *)vector {
    x = [vector x];
    y = [vector y];
    z = [vector z];
    w = 1;
}

- (void)setVector4f:(Vector4f *)vector {
    x = [vector x];
    y = [vector y];
    z = [vector z];
    w = [vector w];
}

- (void)getVector3f:(Vector3f *)vector {
    [vector setX:x / w];
    [vector setY:y / w];
    [vector setZ:z / w];
}

- (BOOL)isNull {
    return fzero(x) && fzero(y) && fzero(z) && fzero(w);
}

- (void)scale:(float)f {
    x *= f;
    y *= f;
    z *= f;
    w *= f;
}

- (void)normalize {
    [self scale:1 / [self length]];
}

- (float)length {
    return sqrtf([self lengthSquared]);
}

- (float)lengthSquared {
    return x * x + y * y + z * z + w * w;
}

- (BOOL)isEqualToVector:(Vector4f *)vector {
    if ([self isEqual:vector])
        return YES;
    
    return feq(x, [vector x]) && feq(y, [vector y]) && feq(z, [vector z]) && feq(w, [vector w]);
}

@end

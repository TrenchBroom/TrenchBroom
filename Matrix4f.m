//
//  Matrix4f.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Matrix4f.h"
#import "Vector3f.h"
#import "Vector4f.h"

@implementation Matrix4f

- (id)init {
    if (self = [super init]) {
        values = malloc(4 * 4 * sizeof(float));
        [self setIdentity];
    }
    
    return self;
}

- (void)setIdentity {
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            if (c == r)
                values[c * 4 + r] = 1;
            else
                values[c * 4 + r] = 0;
}

- (void)setMatrix4f:(Matrix4f *)matrix {
    memcpy(values, [matrix columnMajor], 4 * 4 * sizeof(float));
}

- (void)multiply:(float *)m {
    float* t = malloc(4 * 4 * sizeof(float));
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            t[c * 4 + r] = 0;
            for (int i = 0; i < 3; i++)
                t[c * 4 + r] += values[i * 4 + r] * m[c * 4 + i];
        }
    }
    
    free(values);
    values = t;
}

- (void)rotateAbout:(Vector3f *)axis angle:(float)a {
    if (axis == nil)
        [NSException raise:NSInvalidArgumentException format:@"axis must not be nil"];
    
    float s = sin(a);
    float c = cos(a);
    float t = 1 - c;
    
    float tx  = t  * [axis x];
    float tx2 = tx * [axis x];
    float txy = tx * [axis y];
    float txz = tx * [axis z];
    
    float ty  = t  * [axis y];
    float ty2 = ty * [axis y];
    float tyz = ty * [axis z];
    
    float tz2 = t  * [axis z] * [axis z];

    float sx = s * [axis x];
    float sy = s * [axis y];
    float sz = s * [axis z];
    
    float r[4 * 4];
    r[0] = tx2 + c;
    r[1] = txy - sz;
    r[2] = txz + sy;
    r[3] = 0;
    
    r[4] = txy + sz;
    r[5] = ty2 + c;
    r[6] = tyz - sx;
    r[7] = 0;
    
    r[8] = txz - sy;
    r[9] = tyz + sx;
    r[10] = tz2 + c;
    r[11] = 0;
    
    r[12] = 0;
    r[13] = 0;
    r[14] = 0;
    r[15] = 1;
    
    [self multiply:r];
}

- (void)translate:(Vector3f *)offset {
    if (offset == nil)
        [NSException raise:NSInvalidArgumentException format:@"offset must not be nil"];
    
    values[13] += [offset x];
    values[14] += [offset y];
    values[15] += [offset z];
}

- (void)transformVector3f:(Vector3f *)vector {
}

- (void)transformVector4f:(Vector4f *)vector {
    
}

- (float*)columnMajor {
    return values;
}

- (void)dealloc {
    free(values);
    [super dealloc];
}

@end

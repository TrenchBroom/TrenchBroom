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
#import "Matrix3f.h"
#import "Matrix2f.h"
#import "Math.h"

@implementation Matrix4f

- (id)init {
    if (self = [super init]) {
        values = malloc(4 * 4 * sizeof(float));
        [self setIdentity];
    }
    
    return self;
}

- (id)initWithMatrix4f:(Matrix4f *)matrix{
    if (self = [self init]) {
        [self setMatrix4f:matrix];
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

- (void)setSubMatrix:(int)index to:(Matrix2f *)matrix {
    if (matrix == nil)
        [NSException raise:NSInvalidArgumentException format:@"matrix must not be nil"];
    
    float* mvalues = [matrix columnMajor];
    switch (index) {
        case 0:
            values[ 0] = mvalues[0];
            values[ 1] = mvalues[1];
            values[ 4] = mvalues[2];
            values[ 5] = mvalues[3];
            break;
        case 1:
            values[ 2] = mvalues[0];
            values[ 3] = mvalues[1];
            values[ 6] = mvalues[2];
            values[ 7] = mvalues[3];
            break;
        case 2:
            values[ 8] = mvalues[0];
            values[ 9] = mvalues[1];
            values[12] = mvalues[2];
            values[13] = mvalues[3];
            break;
        case 3:
            values[10] = mvalues[0];
            values[11] = mvalues[1];
            values[14] = mvalues[2];
            values[15] = mvalues[3];
            break;
        default:
            [NSException raise:NSInvalidArgumentException format:@"sub matrix index out of bounds: %i", index];
            break;
    }
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

- (BOOL)invert {
    float det = [self determinant];
    if (fzero(det))
        return NO;
    
    Matrix2f* A = [[Matrix2f alloc] initAsSubMatrix:0 of:self];
    Matrix2f* Ai = [[Matrix2f alloc] initWithMatrix2f:A];
    if ([Ai invert]) { // use quick method
        Matrix2f* B = [[Matrix2f alloc] initAsSubMatrix:2 of:self];
        Matrix2f* C = [[Matrix2f alloc] initAsSubMatrix:1 of:self];
        Matrix2f* D = [[Matrix2f alloc] initAsSubMatrix:3 of:self];
        
        Matrix2f* CAi = [[Matrix2f alloc] initWithMatrix2f:C];
        [CAi mul:Ai];
        
        Matrix2f* CAiB = [[Matrix2f alloc] initWithMatrix2f:CAi];
        [CAiB mul:B];
        
        Matrix2f* AiB = [[Matrix2f alloc] initWithMatrix2f:Ai];
        [AiB mul:B];
        
        [D sub:CAiB];
        
        [A setMatrix2f:AiB];
        [A mul:D];
        
        [B setMatrix2f:A];
        [B negate];
        
        [A mul:CAi];
        [A add:Ai];
        
        [C setMatrix2f:D];
        [C mul:CAi];
        [C negate];
        
        [self setSubMatrix:0 to:A];
        [self setSubMatrix:1 to:C];
        [self setSubMatrix:2 to:B];
        [self setSubMatrix:3 to:D];
        
        [B release];
        [C release];
        [D release];
        
        [CAi release];
        [CAiB release];
        [AiB release];
    } else { // use general but slower method
        [self adjunct];
        [self scale:1 / det];
    }
    
    [A release];
    [Ai release];
    return YES;
}

- (void)adjunct {
    float* nvalues = malloc(4 * 4 * sizeof(float));
    Matrix3f* m = [[Matrix3f alloc] init];
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++) {
            [m setMinorOf:self col:col row:row];
            nvalues[col * 4 + row] = [m determinant];
        }
    
    free(values);
    values = nvalues;
    [m release];
}

- (float)determinant {
    return values[ 0] * values[ 5] * values[10] * values[15]
         + values[ 4] * values[ 9] * values[14] * values[ 3]
         + values[ 8] * values[13] * values[ 2] * values[ 7]
         + values[12] * values[ 1] * values[ 6] * values[11]
         - values[ 3] * values[ 6] * values[ 9] * values[12]
         - values[ 7] * values[10] * values[13] * values[ 0]
         - values[11] * values[14] * values[ 1] * values[ 4]
         - values[15] * values[ 2] * values[ 5] * values[ 8];
}

- (void)setColumn:(int)col row:(int)row value:(float)value {
    if (col < 0 || col > 3)
        [NSException raise:NSInvalidArgumentException format:@"column index out of bounds: %i", col];
    if (row < 0 || row > 3)
        [NSException raise:NSInvalidArgumentException format:@"row index out of bounds: %i", row];
    
    values[col * 4 + row] = value;
}

- (void)setRow:(int)row values:(Vector3f *)vector {
    if (row < 0 || row > 3)
        [NSException raise:NSInvalidArgumentException format:@"row index out of bounds: %i", row];
    if (vector == nil)
        [NSException raise:NSInvalidArgumentException format:@"vector must not be nil"];
    
    values[ 0 + row] = [vector x];
    values[ 4 + row] = [vector y];
    values[ 8 + row] = [vector z];
    values[12 + row] = 0;
}

- (void)setColumn:(int)col values:(Vector3f *)vector {
    if (col < 0 || col > 3)
        [NSException raise:NSInvalidArgumentException format:@"column index out of bounds: %i", col];
    if (vector == nil)
        [NSException raise:NSInvalidArgumentException format:@"vector must not be nil"];
    
    values[col * 4 + 0] = [vector x];
    values[col * 4 + 1] = [vector y];
    values[col * 4 + 2] = [vector z];
    values[col * 4 + 3] = 0;
}

- (void)transformVector3f:(Vector3f *)vector {
    Vector4f* vector4f = [[Vector4f alloc] initWithVector3f:vector];
    [self transformVector4f:vector4f];
    [vector4f getVector3f:vector];
    [vector4f release];
}

- (void)transformVector4f:(Vector4f *)vector {
    if (vector == nil)
        [NSException raise:NSInvalidArgumentException format:@"vector must not be nil"];
    
    float x = values[ 0] * [vector x] + values[ 4] * [vector y] + values[ 8] * [vector z] + values[12] * [vector w];
    float y = values[ 1] * [vector x] + values[ 5] * [vector y] + values[ 9] * [vector z] + values[13] * [vector w];
    float z = values[ 2] * [vector x] + values[ 6] * [vector y] + values[10] * [vector z] + values[14] * [vector w];
    float w = values[ 3] * [vector x] + values[ 7] * [vector y] + values[11] * [vector z] + values[15] * [vector w];
    
    [vector setX:x];
    [vector setY:y];
    [vector setZ:z];
    [vector setW:w];
}

- (void)negate {
    for (int i = 0; i < 16; i++)
        values[i] = -1 * values[i];
}

- (void)add:(Matrix4f *)matrix {
    float* mvalues = [matrix columnMajor];
    for (int i = 0; i < 16; i++)
        values[i] += mvalues[i];
}

- (void)sub:(Matrix4f *)matrix {
    float* mvalues = [matrix columnMajor];
    for (int i = 0; i < 16; i++)
        values[i] -= mvalues[i];
}

- (void)mul:(Matrix4f *)matrix {
    float* mvalues = [matrix columnMajor];
    float* nvalues = malloc(4 * 4 * sizeof(float));
    
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            nvalues[col * 4 + row] = 0;
            for (int i = 0; i < 4; i++)
                nvalues[col * 4 + row] += values[i * 4 + row] * mvalues[col * 4 + i];
        }
    }
    
    free(values);
    values = nvalues;
}

- (void)scale:(float)factor {
    for (int i = 0; i < 16; i++)
        values[i] *= factor;
}

- (float*)columnMajor {
    return values;
}

- (NSString *)description {
    NSMutableString* desc = [NSMutableString string];
    [desc appendFormat:@"%f %f %f %f\n", values[ 0], values[ 4], values[ 8], values[12]];
    [desc appendFormat:@"%f %f %f %f\n", values[ 1], values[ 5], values[ 9], values[13]];
    [desc appendFormat:@"%f %f %f %f\n", values[ 2], values[ 6], values[10], values[14]];
    [desc appendFormat:@"%f %f %f %f\n", values[ 3], values[ 7], values[11], values[15]];
    
    return desc;
}

- (void)dealloc {
    free(values);
    [super dealloc];
}

@end

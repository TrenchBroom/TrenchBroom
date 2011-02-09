//
//  Matrix2f.m
//  TrenchBroom
//
//  Created by Kristian Duske on 09.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Matrix2f.h"
#import "Matrix4f.h"
#import "Matrix3f.h"
#import "Math.h"

@implementation Matrix2f

- (id)init {
    if (self = [super init]) {
        values = malloc(2 * 2 * sizeof(float));
        [self setIdentity];
    }
    
    return self;
}

- (id)initWithMatrix2f:(Matrix2f *)matrix {
    if (self = [self init]) {
        [self setMatrix2f:matrix];
    }
    
    return self;
}

- (id)initAsSubMatrix:(int)index of:(Matrix4f *)matrix {
    if (matrix == nil)
        [NSException raise:NSInvalidArgumentException format:@"matrix must not be nil"];
    
    if (self = [self init]) {
        float* mvalues = [matrix columnMajor];
        switch (index) {
            case 0:
                values[0] = mvalues[0];
                values[1] = mvalues[1];
                values[2] = mvalues[4];
                values[3] = mvalues[5];
                break;
            case 1:
                values[0] = mvalues[2];
                values[1] = mvalues[3];
                values[2] = mvalues[6];
                values[3] = mvalues[7];
                break;
            case 2:
                values[0] = mvalues[8];
                values[1] = mvalues[9];
                values[2] = mvalues[12];
                values[3] = mvalues[13];
                break;
            case 3:
                values[0] = mvalues[10];
                values[1] = mvalues[11];
                values[2] = mvalues[14];
                values[3] = mvalues[15];
                break;
            default:
                [NSException raise:NSInvalidArgumentException format:@"sub matrix index out of bounds: %i", index];
                break;
        }
    }
    
    return self;
}

- (void)setMinorOf:(Matrix3f *)matrix col:(int)col row:(int)row {
    if (matrix == nil)
        [NSException raise:NSInvalidArgumentException format:@"matrix must not be nil"];
    
    float* mvalues = [matrix columnMajor];
    int i = 0;
    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            if (c != col && r != row)
                values[i++] = mvalues[c * 3 + r];
}

- (void)setIdentity {
    values[0] = 1;
    values[1] = 0;
    values[2] = 0;
    values[3] = 1;
}

- (void)setMatrix2f:(Matrix2f *)matrix {
    memcpy(values, [matrix columnMajor], 2 * 2 * sizeof(float));
}

- (BOOL)invert {
    float det = [self determinant];
    if (fzero(det))
        return NO;
    
    [self adjunct];
    [self scale:1 / det];
    return YES;
}

- (void)adjunct {
    float t = values[0];
    values[0] = values[3];
    values[3] = t;
    
    values[1] *= -1;
    values[2] *= -1;
}

- (float)determinant {
    return values[0] * values[3] - values[2] * values[1];
}

- (void)negate {
    for (int i = 0; i < 4; i++)
        values[i] = -1 * values[i];
}

- (void)add:(Matrix2f *)matrix {
    float* mvalues = [matrix columnMajor];
    for (int i = 0; i < 4; i++)
        values[i] += mvalues[i];
}

- (void)sub:(Matrix2f *)matrix {
    float* mvalues = [matrix columnMajor];
    for (int i = 0; i < 4; i++)
        values[i] -= mvalues[i];
}

- (void)mul:(Matrix2f *)matrix {
    float* mvalues = [matrix columnMajor];
    float a = values[0] * mvalues[0] + values[2] * mvalues[1];
    float c = values[1] * mvalues[0] + values[3] * mvalues[1];
    float b = values[0] * mvalues[2] + values[2] * mvalues[3];
    float d = values[1] * mvalues[2] + values[3] * mvalues[3];
    
    values[0] = a;
    values[1] = c;
    values[2] = b;
    values[3] = d;
}

- (void)scale:(float)factor {
    for (int i = 0; i < 4; i++)
        values[i] *= factor;
}

- (float*)columnMajor {
    return values;
}

- (NSString *)description {
    NSMutableString* desc = [[NSMutableString alloc] init];
    
    [desc appendFormat:@"%f %f\n", values[ 0], values[ 2]];
    [desc appendFormat:@"%f %f\n", values[ 1], values[ 3]];
    
    return [desc autorelease];
}

- (void)dealloc {
    free(values);
    [super dealloc];
}

@end

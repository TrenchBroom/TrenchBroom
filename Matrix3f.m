//
//  Matrix3f.m
//  TrenchBroom
//
//  Created by Kristian Duske on 09.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Matrix3f.h"
#import "Matrix4f.h"
#import "Matrix2f.h"
#import "Math.h"

@implementation Matrix3f

- (id)init {
    if (self = [super init]) {
        values = malloc(3 * 3 * sizeof(float));
        [self setIdentity];
    }
    
    return self;
}

- (id)initWithMatrix3f:(Matrix3f *)matrix {
    if (self = [self init]) {
        [self setMatrix3f:matrix];
    }
    
    return self;
}

- (id)initAsMinorCol:(int)col row:(int)row of:(Matrix4f *)matrix {
    if (matrix == nil)
        [NSException raise:NSInvalidArgumentException format:@"matrix must not be nil"];
    
    if (self = [self init]) {
        float* mvalues = [matrix columnMajor];
        int i = 0;
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                if (c != col && r != row)
                    values[i++] = mvalues[c * 4 + r];
    }
    
    return self;
}

- (void)setIdentity {
    for (int row = 0; row < 3; row++)
        for (int col = 0; col < 3; col++)
            values[col * 3 + row] = row == col ? 1 : 0;
}

- (void)setMatrix3f:(Matrix3f *)matrix {
    memcpy(values, [matrix columnMajor], 3 * 3 * sizeof(float));
}

- (void)setMinorOf:(Matrix4f *)matrix col:(int)col row:(int)row {
    if (matrix == nil)
        [NSException raise:NSInvalidArgumentException format:@"matrix must not be nil"];
    
    float* mvalues = [matrix columnMajor];
    int i = 0;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            if (c != col && r != row)
                values[i++] = mvalues[c * 4 + r];
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
    float* nvalues = malloc(3 * 3 * sizeof(float));
    Matrix2f* m = [[Matrix2f alloc] init];
    for (int col = 0; col < 3; col++)
        for (int row = 0; row < 3; row++) {
            [m setMinorOf:self col:col row:row];
            nvalues[col * 3 + row] = [m determinant];
        }
            
    free(values);
    values = nvalues;
    [m release];
}

- (float)determinant {
    return values[0] * values[4] * values[8]
         + values[3] * values[7] * values[1]
         + values[6] * values[1] * values[5]
         - values[2] * values[4] * values[6]
         - values[5] * values[7] * values[0]
         - values[8] * values[1] * values[3];
}

- (void)negate {
    for (int i = 0; i < 9; i++)
        values[i] = -1 * values[i];
}

- (void)add:(Matrix3f *)matrix {
    float* mvalues = [matrix columnMajor];
    for (int i = 0; i < 9; i++)
        values[i] += mvalues[i];
}

- (void)sub:(Matrix3f *)matrix {
    float* mvalues = [matrix columnMajor];
    for (int i = 0; i < 9; i++)
        values[i] -= mvalues[i];
}

- (void)mul:(Matrix3f *)matrix {
    float* mvalues = [matrix columnMajor];
    float* nvalues = malloc(3 * 3 * sizeof(float));
    
    for (int col = 0; col < 3; col++) {
        for (int row = 0; row < 3; row++) {
            nvalues[col * 3 + row] = 0;
            for (int i = 0; i < 3; i++)
                nvalues[col * 3 + row] += values[i * 3 + row] * mvalues[col * 3 + i];
        }
    }

    free(values);
    values = nvalues;
}

- (void)scale:(float)factor {
    for (int i = 0; i < 9; i++)
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

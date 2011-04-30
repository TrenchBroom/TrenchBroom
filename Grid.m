//
//  Grid.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Grid.h"
#import "math.h"
#import "FloatData.h"

NSString* const GridChanged = @"GridChanged";

@implementation Grid

- (id)init {
    if (self = [super init]) {
        size = 4;
        draw = YES;
        snap = YES;
        for (int i = 0; i < 9; i++)
            texIds[i] = 0;
    }
    
    return self;
}

- (int)size {
    return size;
}

- (int)actualSize {
    return 1 << size;
}

- (BOOL)draw {
    return draw;
}

- (BOOL)snap {
    return snap;
}

- (void)setSize:(int)theSize {
    if (theSize < 0 || theSize > 8)
        [NSException raise:NSInvalidArgumentException format:@"invalid grid size: %i", theSize];

    if (size == theSize)
        return;
    
    size = theSize;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:GridChanged object:self];
}

- (void)setDraw:(BOOL)isDrawEnabled {
    if (draw == isDrawEnabled)
        return;
    
    draw = isDrawEnabled;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:GridChanged object:self];
}

- (void)setSnap:(BOOL)isSnapEnabled {
    if (snap == isSnapEnabled)
        return;
    
    snap = isSnapEnabled;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:GridChanged object:self];
}

- (void)toggleDraw {
    [self setDraw:![self draw]];
}

- (void)toggleSnap {
    [self setSnap:![self snap]];
}

- (void)snapToGrid:(TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * roundf(vector->x / actualSize);
    result->y = actualSize * roundf(vector->y / actualSize);
    result->z = actualSize * roundf(vector->z / actualSize);
}

- (void)snapUpToGrid:(TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * ceilf(vector->x / actualSize);
    result->y = actualSize * ceilf(vector->y / actualSize);
    result->z = actualSize * ceilf(vector->z / actualSize);
}

- (void)snapDownToGrid:(TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * floorf(vector->x / actualSize);
    result->y = actualSize * floorf(vector->y / actualSize);
    result->z = actualSize * floorf(vector->z / actualSize);
}

- (void)gridOffsetOf:(TVector3f *)vector result:(TVector3f *)result {
    TVector3f snapped;
    [self snapToGrid:vector result:&snapped];
    subV3f(vector, &snapped, result);
}

- (void)activateTexture {
    if (texIds[size] == 0) {
        glGenTextures(1, &texIds[size]);
        
        int dim = [self actualSize];
        if (dim < 4)
            dim = 4;
        int texSize = 1 << 8; // 256 biggest grid size
        char* pixel = malloc(texSize * texSize * 4);
            for (int y = 0; y < texSize; y++)
                for (int x = 0; x < texSize; x++) {
                    int i = (y * texSize + x) * 4;
                    if ((x % dim) == 0 || (y % dim) == 0) {
                        pixel[i + 0] = 0xFF;
                        pixel[i + 1] = 0xFF;
                        pixel[i + 2] = 0xFF;
                        pixel[i + 3] = 0x22;
                    } else {
                        pixel[i + 0] = 0x00;
                        pixel[i + 1] = 0x00;
                        pixel[i + 2] = 0x00;
                        pixel[i + 3] = 0x00;
                    }
                }
        
        glBindTexture(GL_TEXTURE_2D, texIds[size]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        free(pixel);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, texIds[size]);
    }
}

- (void)deactivateTexture {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (void)dealloc {
    for (int i = 0; i < 9; i++)
        glDeleteTextures(1, &texIds[i]);
    [super dealloc];
}

@end

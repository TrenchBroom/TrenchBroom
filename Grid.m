//
//  Grid.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Grid.h"
#import "Vector3f.h"
#import "math.h"
#import "FloatData.h"

NSString* const GridChanged = @"GridChanged";

@implementation Grid

- (id)init {
    if (self = [super init]) {
        size = 1;
        draw = YES;
        snap = YES;
        for (int i = 0; i < 6; i++)
            texIds[i] = 0;
    }
    
    return self;
}

- (int)size {
    return size;
}

- (int)actualSize {
    return 1 << (size + 3);
}

- (BOOL)draw {
    return draw;
}

- (BOOL)snap {
    return snap;
}

- (void)setSize:(int)theSize {
    if (theSize < 0 || theSize > 5)
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

- (void)snapToGrid:(Vector3f *)vector {
    [vector setX:[self actualSize] * roundf([vector x] / [self actualSize])];
    [vector setY:[self actualSize] * roundf([vector y] / [self actualSize])];
    [vector setZ:[self actualSize] * roundf([vector z] / [self actualSize])];
}

- (void)snapUpToGrid:(Vector3f *)vector {
    [vector setX:[self actualSize] * ceilf([vector x] / [self actualSize])];
    [vector setY:[self actualSize] * ceilf([vector y] / [self actualSize])];
    [vector setZ:[self actualSize] * ceilf([vector z] / [self actualSize])];
}

- (void)snapDownToGrid:(Vector3f *)vector {
    [vector setX:[self actualSize] * floorf([vector x] / [self actualSize])];
    [vector setY:[self actualSize] * floorf([vector y] / [self actualSize])];
    [vector setZ:[self actualSize] * floorf([vector z] / [self actualSize])];
}

- (Vector3f *)gridOffsetOf:(Vector3f *)vector {
    Vector3f* snapped = [[Vector3f alloc] initWithFloatVector:vector];
    [self snapToGrid:snapped];
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:vector];
    [diff sub:snapped];

    [snapped release];
    return [diff autorelease];
}

- (void)activateTexture {
    if (texIds[size] == 0) {
        glGenTextures(1, &texIds[size]);
        
        int dim = [self actualSize];
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
    for (int i = 0; i < 6; i++)
        glDeleteTextures(1, &texIds[i]);
    [super dealloc];
}

@end

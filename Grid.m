/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "Grid.h"
#import "math.h"
#import "FloatData.h"

NSString* const GridChanged = @"GridChanged";

@implementation Grid

- (id)init {
    if ((self = [super init])) {
        size = 4;
        alpha = 0.1f;
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

- (float)alpha {
    return alpha;
}

- (int)actualSize {
    if (snap)
        return 1 << size;
    return 1;
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
    
    [super willChangeValueForKey:@"actualSize"];
    size = theSize;
    [super didChangeValueForKey:@"actualSize"];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:GridChanged object:self];
}

- (void)setAlpha:(float)theAlpha {
    if (alpha == theAlpha)
        return;
    
    alpha = theAlpha;
    for (int i = 0; i < 9; i++)
        valid[i] = NO;
    
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

- (float)snapToGridf:(float)f {
    int actualSize = [self actualSize];
    return actualSize * roundf(f / actualSize);
}

- (float)snapUpToGridf:(float)f {
    int actualSize = [self actualSize];
    return actualSize * ceilf(f / actualSize);
}

- (float)snapDownToGridf:(float)f {
    int actualSize = [self actualSize];
    return actualSize * floorf(f / actualSize);
}

- (void)snapToGridV3f:(const TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * roundf(vector->x / actualSize);
    result->y = actualSize * roundf(vector->y / actualSize);
    result->z = actualSize * roundf(vector->z / actualSize);
}

- (void)snapToFarthestGridV3f:(const TVector3f *)vector result:(TVector3f *)result {
    TVector3f ceil, floor;
    [self snapUpToGridV3f:vector result:&ceil];
    [self snapDownToGridV3f:vector result:&floor];

    result->x = vector->x < 0 ? floor.x : ceil.x;
    result->y = vector->y < 0 ? floor.y : ceil.y;
    result->z = vector->z < 0 ? floor.z : ceil.z;
}

- (void)snapUpToGridV3f:(const TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * ceilf(vector->x / actualSize);
    result->y = actualSize * ceilf(vector->y / actualSize);
    result->z = actualSize * ceilf(vector->z / actualSize);
}

- (void)snapDownToGridV3f:(const TVector3f *)vector result:(TVector3f *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * floorf(vector->x / actualSize);
    result->y = actualSize * floorf(vector->y / actualSize);
    result->z = actualSize * floorf(vector->z / actualSize);
}

- (void)gridOffsetV3f:(const TVector3f *)vector result:(TVector3f *)result {
    TVector3f snapped;
    [self snapToGridV3f:vector result:&snapped];
    subV3f(vector, &snapped, result);
}

- (void)snapToGridV3i:(const TVector3i *)vector result:(TVector3i *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * roundf(vector->x / (float)actualSize);
    result->y = actualSize * roundf(vector->y / (float)actualSize);
    result->z = actualSize * roundf(vector->z / (float)actualSize);
}

- (void)snapUpToGridV3i:(const TVector3i *)vector result:(TVector3i *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * ceilf(vector->x / (float)actualSize);
    result->y = actualSize * ceilf(vector->y / (float)actualSize);
    result->z = actualSize * ceilf(vector->z / (float)actualSize);
}

- (void)snapDownToGridV3i:(const TVector3i *)vector result:(TVector3i *)result {
    int actualSize = [self actualSize];
    result->x = actualSize * floorf(vector->x / (float)actualSize);
    result->y = actualSize * floorf(vector->y / (float)actualSize);
    result->z = actualSize * floorf(vector->z / (float)actualSize);
}

- (void)snapToGridV3i:(const TVector3i *)vector direction:(TVector3f *)direction result:(TVector3i *)result {
    int actualSize = [self actualSize];
    if (direction->x >= 0)
        result->x = actualSize * ceilf(vector->x / (float)actualSize);
    else
        result->x = actualSize * floorf(vector->x / (float)actualSize);
    if (direction->y >= 0)
        result->y = actualSize * ceilf(vector->y / (float)actualSize);
    else
        result->y = actualSize * floorf(vector->y / (float)actualSize);
    if (direction->z >= 0)
        result->z = actualSize * ceilf(vector->z / (float)actualSize);
    else
        result->z = actualSize * floorf(vector->z / (float)actualSize);
}

- (void)gridOffsetV3i:(const TVector3i *)vector result:(TVector3i *)result {
    TVector3i snapped;
    [self snapToGridV3i:vector result:&snapped];
    subV3i(vector, &snapped, result);
}

- (void)activateTexture {
    if (!valid[size]) {
        if (texIds[size] == 0)
            glGenTextures(1, &texIds[size]);
        
        int dim = [self actualSize];
        if (dim < 4)
            dim = 4;
        int texSize = 1 << 8; // 256 biggest grid size
        char pixel[texSize * texSize * 4];
            for (int y = 0; y < texSize; y++)
                for (int x = 0; x < texSize; x++) {
                    int i = (y * texSize + x) * 4;
                    if ((x % dim) == 0 || (y % dim) == 0) {
                        pixel[i + 0] = 0xFF;
                        pixel[i + 1] = 0xFF;
                        pixel[i + 2] = 0xFF;
                        pixel[i + 3] = 0xFF * alpha;
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
        valid[size] = YES;
    } else {
        glBindTexture(GL_TEXTURE_2D, texIds[size]);
    }
}

- (void)deactivateTexture {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (void)dealloc {
    for (int i = 0; i < 9; i++)
        if (texIds[i] != 0)
            glDeleteTextures(1, &texIds[i]);
    [super dealloc];
}

@end

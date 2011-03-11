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
#import "MathCache.h"

NSString* const GridChanged = @"GridChanged";

@implementation Grid

- (id)init {
    if (self = [super init]) {
        size = 16;
        draw = YES;
        snap = YES;
    }
    
    return self;
}

- (int)size {
    return size;
}

- (BOOL)draw {
    return draw;
}

- (BOOL)snap {
    return snap;
}

- (void)setSize:(int)theSize {
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
    [vector setX:size * roundf([vector x] / size)];
    [vector setY:size * roundf([vector y] / size)];
    [vector setZ:size * roundf([vector z] / size)];
}

- (Vector3f *)gridOffsetOf:(Vector3f *)vector {
    MathCache* cache = [MathCache sharedCache];
    Vector3f* snapped = [cache vector3f];
    [snapped setFloat:vector];
    [self snapToGrid:snapped];
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:vector];
    [diff sub:snapped];
    
    [cache returnVector3f:snapped];
    return [diff autorelease];
}

@end

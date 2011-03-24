//
//  Math.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Math.h"
#import "math.h"
#import "Vector2f.h"
#import "Vector3f.h"

float const AlmostZero = 0.001f;

BOOL fzero(float v) {
    return fabsf(v) <= AlmostZero;
}

BOOL fpos(float v) {
    return v > AlmostZero;
}

BOOL fneg(float v) {
    return v < -AlmostZero;
}

BOOL feq(float v1, float v2) {
    return fabsf(v1 - v2) < AlmostZero;
}

BOOL fgt(float v1, float v2) {
    return v1 > v2 + AlmostZero;
}

BOOL flt(float v1, float v2) {
    return v1 < v2 - AlmostZero;
}

BOOL fgte(float v1, float v2) {
    return !flt(v1, v2);
}

BOOL flte(float v1, float v2) {
    return !fgt(v1, v2);
}
             
BOOL finxx(float v, float b1, float b2) {
    return b1 < b2 ? fgt(v, b1) && flt(v, b2) : fgt(v, b2) && flt(v, b1);
}

BOOL finxi(float v, float b1, float b2) {
    return b1 < b2 ? fgt(v, b1) && flte(v, b2) : fgt(v, b2) && flte(v, b1);
}

BOOL finix(float v, float b1, float b2) {
    return b1 < b2 ? fgte(v, b1) && flt(v, b2) : fgte(v, b2) && flt(v, b1);
}

BOOL finii(float v, float b1, float b2) {
    return b1 < b2 ? fgte(v, b1) && flte(v, b2) : fgte(v, b2) && flte(v, b1);
}

NSArray* makeCircle(float radius, int segments) {
    NSMutableArray* points = [[NSMutableArray alloc] initWithCapacity:segments];
    
    float d = 2 * M_PI / segments;
    float a = 0;
    for (int i = 0; i < segments; i++) {
        float s = sin(a);
        float c = cos(a);
        Vector3f* point = [[Vector3f alloc] initWithX:radius * s y:radius * c z:0];
        [points addObject:point];
        [point release];
        a += d;
    }
    
    return [points autorelease];
}

NSArray* makeRing(float innerRadius, float outerRadius, int segments) {
    NSMutableArray* points = [[NSMutableArray alloc] initWithCapacity:2 * segments + 2];
    
    float d = 2 * M_PI / (2 * segments);
    float a = 0;
    for (int i = 0; i < 2 * segments; i++) {
        float s = sin(a);
        float c = cos(a);
        float r = i % 2 == 0 ? innerRadius : outerRadius;
        
        Vector3f* point = [[Vector3f alloc] initWithX:r * s y:r * c z:0];
        [points addObject:point];
        [point release];
        a += d;
    }
    
    [points addObject:[points objectAtIndex:0]];
    [points addObject:[points objectAtIndex:1]];
    
    return [points autorelease];
}

int smallestXVertex2D(NSArray *vertices) {
    if (vertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
    if ([vertices count] == 0)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be empty"];
        
    int s = 0;
    Vector3f* v = [vertices objectAtIndex:0];
    float x = [v x];
    float y = [v y];
    
    for (int i = 1; i < [vertices count]; i++) {
        v = [vertices objectAtIndex:i];
        if (flt([v x], x) || (feq([v x], x) && flt([v y], y))) {
            x = [v x];
            y = [v y];
            s = i;
        }
    }
    
    return s;
}

int smallestYVertex2D(NSArray *vertices)  {
    if (vertices == nil)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be nil"];
    if ([vertices count] == 0)
        [NSException raise:NSInvalidArgumentException format:@"vertex array must not be empty"];
    
    int s = 0;
    Vector3f* v = [vertices objectAtIndex:0];
    float x = [v x];
    float y = [v y];
    
    for (int i = 1; i < [vertices count]; i++) {
        v = [vertices objectAtIndex:i];
        if (flt([v y], y) || (feq([v y], y) && flt([v x], x))) {
            x = [v x];
            y = [v y];
            s = i;
        }
    }
    
    return s;
}

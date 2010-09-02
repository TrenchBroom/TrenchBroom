//
//  Edge2D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Edge2D.h"
#import "Math.h"
#import "Vector2f.h"

@implementation Edge2D

- (id)initWithLine:(Line2D *)l normal:(Vector2f *)o {
    return [self initWithLine:l previous:nil next:nil normal:o];
}

- (id)initWithLine:(Line2D *)l previous:(Edge2D *)p normal:(Vector2f *)o {
    return [self initWithLine:l previous:p next:nil normal:o];
}

- (id)initWithLine:(Line2D *)l next:(Edge2D *)n normal:(Vector2f *)o {
    return [self initWithLine:l previous:nil next:n normal:o];
}

- (id)initWithLine:(Line2D *)l previous:(Edge2D *)p next:(Edge2D *)n normal:(Vector2f *)o {
    if (!l)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    if (!o)
        [NSException raise:NSInvalidArgumentException format:@"normal must not be nil"];

    if (self = [super init]) {
        line = [[Line2D alloc] initWithLine:(Line2D *)l];
        [self setPrevious:p];
        [self setNext:n];
        startVertex = nil;
        normal = [[Vector2f alloc] initWithVector:o];
    }
    
    return self;
}

- (id)initWithStart:(Vector2f *)s end:(Vector2f *)e {
    return [self initWithStart:s end:e previous:nil next:nil];
}

- (id)initWithStart:(Vector2f *)s end:(Vector2f *)e previous:(Edge2D *)p {
    return [self initWithStart:s end:e previous:p next:nil];
}

- (id)initWithStart:(Vector2f *)s end:(Vector2f *)e next:(Edge2D *)n {
    return [self initWithStart:s end:e previous:nil next:n];
}

- (id)initWithStart:(Vector2f *)s end:(Vector2f *)e previous:(Edge2D *)p next:(Edge2D *)n {
    if (!s)
        [NSException raise:NSInvalidArgumentException format:@"start must not be nil"];
    if (!e)
        [NSException raise:NSInvalidArgumentException format:@"end must not be nil"];
    if ([s isEqual:e])
        [NSException raise:NSInvalidArgumentException format:@"start and end must not be identical"];

    Line2D* l = [[Line2D alloc] initWithPoint1:s point2:e];
    Vector2f* o = [[Vector2f alloc] initWithVector:e];
    [o sub:s];
    [o normalize];
    
    self = [self initWithLine:l previous:p next:n normal:o];
    [l release];
    [o release];
    
    return self;
}


- (Vector2f *)startVertex {
    if (!startVertex && previous) {
        Line2D* l = [previous line];
        startVertex = [l intersectionWithLine:line];
        [startVertex retain];
    }
    
    return startVertex;
}

- (Vector2f *)endVertex {
    if (!next)
        return nil;
    
    return [next startVertex];
}

- (Vector2f *)smallVertex {
    if ([self isUpper])
        return [self startVertex];
    return [self endVertex];
}

- (Vector2f *)largeVertex {
    if ([self isUpper])
        return [self endVertex];
    return [self startVertex];
}


- (Line2D *)line {
    return line;
}

- (Vector2f *)normal {
    return normal;
}

- (BOOL)isUpper {
    if ([normal y] > AlmostZero)
        return YES;
    return fabsf([normal y]) <= AlmostZero && [normal x] > AlmostZero;
}

- (BOOL)isLower {
    return ![self isUpper];
}

- (BOOL)containsX:(float)x {
    Vector2f* sv = [self smallVertex];
    Vector2f* lv = [self largeVertex];
    
    if (sv != nil && lv != nil)
        return x >= [sv x] && x <= [lv x];
    else if (sv != nil)
        return x >= [sv x];
    else if (lv != nil)
        return x <= [lv x];

    return YES;
}

- (BOOL)contains:(Vector2f *)p {
    float y = [line yAt:[p x]];
    if ([self isUpper])
        return [p y] <= y;

    return [p y] >= y;
}

- (Vector2f *)intersectWith:(Edge2D *)e {
    Vector2f* is = [line intersectWith:[e line]];
    if (is == nil)
        return nil;
    
    if (![self containsX:[is x]] || ![e containsX:[is x]])
        return nil;
    
    return is;
}

- (void)setPrevious:(Edge2D *)p {
    previous = p;
    [startVertex release];
    startVertex = nil;
}

- (void)setNext:(Edge2D *)n {
    [next release];
    next = [n retain];
}

- (void)open {
    [next setPrevious:nil];
    next = nil; // do not release
}

- (void)close:(Edge2D *)n {
    [next release];
    next = n; // do not retain
    [next setPrevious:self];
}

- (Edge2D *)previous {
    return previous;
}

- (Edge2D *)next {
    return next;
}

- (Edge2D *)insertAfterLine:(Line2D *)l normal:(Vector2f *)o {
    Edge2D* newEdge = [[Edge2D alloc] initWithLine:l previous:self next:next normal:o];
    [next setPrevious:newEdge];
    [self setNext:newEdge];
    
    return [newEdge autorelease];
}

- (Edge2D *)insertAfterStart:(Vector2f *)s end:(Vector2f *)e {
    Edge2D* newEdge = [[Edge2D alloc] initWithStart:s end:s previous:self next:next];
    [next setPrevious:newEdge];
    [self setNext:newEdge];
    
    return [newEdge autorelease];
}

- (Edge2D *)insertBeforeLine:(Line2D *)l normal:(Vector2f *)o {
    Edge* newEdge = [[Edge2D alloc] initWithLine:l previous:previous next:self normal:o];
    [previous setNext:newEdge];
    [self setPrevious:newEdge];
    
    return [newEdge autorelease];
}

- (Edge2D *)insertBeforeStart:(Vector2f *)s end:(Vector2f *)e {
    Edge2D* newEdge = [[Edge2D alloc] initWithStart:s end:e previous:previous next:self];
    [previous setNext:newEdge];
    [self setPrevious:newEdge];
    
    return [newEdge autorelease];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"start: %@, end: %@", [self startVertex], [self endVertex]];
}

- (void)dealloc {
    if (next != nil) {
        [next release];
        next = nil;
    }
    [line release];
    [startVertex release];
    [normal release];
    [super dealloc];
}

@end

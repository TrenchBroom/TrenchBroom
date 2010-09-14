//
//  Edge2D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Edge2D.h"

@implementation Edge2D
- (id)initWithLine:(Line2D *)l norm:(Vector2f *)o {
    return [self initWithLine:l previous:nil next:nil norm:o];
}

- (id)initWithLine:(Line2D *)l previous:(Edge2D *)p norm:(Vector2f *)o {
    return [self initWithLine:l previous:p next:nil norm:o];
}

- (id)initWithLine:(Line2D *)l next:(Edge2D *)n norm:(Vector2f *)o {
    return [self initWithLine:l previous:nil next:n norm:o];
}

- (id)initWithLine:(Line2D *)l previous:(Edge2D *)p next:(Edge2D *)n norm:(Vector2f *)o {
    if (!l)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    if (!o)
        [NSException raise:NSInvalidArgumentException format:@"normal must not be nil"];

    if (self = [super init]) {
        line = [[Line2D alloc] initWithLine:(Line2D *)l];
        [self setPrevious:p];
        [self setNext:n];
        startVertex = nil;
        norm = [[Vector2f alloc] initWithVector:o];
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
    float x = [o x];
    [o setX:[o y]];
    [o setY:-x];
    
    self = [self initWithLine:l previous:p next:n norm:o];
    [l release];
    [o release];
    
    return self;
}


- (Vector2f *)startVertex {
    if (startVertex == nil && previous != nil) {
        Line2D* l = [previous line];
        startVertex = [l intersectWith:line];
        [startVertex retain];
    }
    
    return startVertex;
}

- (Vector2f *)endVertex {
    if (next == nil)
        return nil;
    
    return [next startVertex];
}

- (Vector2f *)smallVertex {
    if ([self isUpper])
        return [self endVertex];
    return [self startVertex];
}

- (Vector2f *)largeVertex {
    if ([self isUpper])
        return [self startVertex];
    return [self endVertex];
}


- (Line2D *)line {
    return line;
}

- (Vector2f *)norm {
    return norm;
}

- (BOOL)isUpper {
    if (fpos([norm y]))
        return YES;
    return fzero([norm y]) && fneg([norm x]);
}

- (BOOL)isLower {
    return ![self isUpper];
}

- (BOOL)contains:(Vector2f *)p {
    float y = [line yAt:[p x]];
    if ([self isUpper])
        return flte([p y], y);

    return fgte([p y], y);
}

- (Vector2f *)intersectWith:(Edge2D *)e {
    Vector2f* is = [line intersectWith:[e line]];
    if (is == nil)
        return nil;
    
    if (!finii([is x], [[self startVertex] x], [[self endVertex] x]) ||
        !finii([is x], [[e startVertex] x], [[e endVertex] x]))
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

- (Edge2D *)insertAfterLine:(Line2D *)l norm:(Vector2f *)o {
    Edge2D* newEdge = [[Edge2D alloc] initWithLine:l previous:self next:next norm:o];
    [next setPrevious:newEdge];
    [self setNext:newEdge];
    
    return [newEdge autorelease];
}

- (Edge2D *)insertAfterStart:(Vector2f *)s end:(Vector2f *)e {
    Edge2D* newEdge = [[Edge2D alloc] initWithStart:s end:e previous:self next:next];
    [next setPrevious:newEdge];
    [self setNext:newEdge];
    
    return [newEdge autorelease];
}

- (Edge2D *)insertBeforeLine:(Line2D *)l norm:(Vector2f *)o {
    Edge2D* newEdge = [[Edge2D alloc] initWithLine:l previous:previous next:self norm:o];
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
    [norm release];
    [super dealloc];
}

@end

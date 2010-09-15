//
//  Edge2D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Edge2D.h"

@implementation Edge2D
+ (Edge2D *)edgeWithBoundary:(Line2D *)boundary outside:(Vector2f *)outside {
    return [[[Edge2D alloc] initWithBoundary:boundary outside:outside] autorelease];
}

+ (Edge2D *)edgeWithBoundary:(Line2D *)boundary previous:(Edge2D *)previousEdge outside:(Vector2f *)outside {
    return [[[Edge2D alloc] initWithBoundary:boundary previous:previousEdge outside:outside] autorelease];
}

+ (Edge2D *)edgeWithBoundary:(Line2D *)boundary next:(Edge2D *)nextEdge outside:(Vector2f *)outside {
    return [[[Edge2D alloc] initWithBoundary:boundary next:nextEdge outside:outside] autorelease];
}

+ (Edge2D *)edgeWithBoundary:(Line2D *)boundary previous:(Edge2D *)previousEdge next:(Edge2D *)nextEdge outside:(Vector2f *)outside {
    return [[[Edge2D alloc] initWithBoundary:boundary previous:previousEdge next:nextEdge outside:outside] autorelease];
}

+ (Edge2D *)edgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex {
    return [[[Edge2D alloc] initWithStart:startVertex end:endVertex] autorelease];
}

+ (Edge2D *)edgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex previous:(Edge2D *)previousEdge {
    return [[[Edge2D alloc] initWithStart:startVertex end:endVertex previous:previousEdge] autorelease];
}

+ (Edge2D *)edgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex next:(Edge2D *)nextEdge {
    return [[[Edge2D alloc] initWithStart:startVertex end:endVertex next:nextEdge] autorelease];
}

+ (Edge2D *)edgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex previous:(Edge2D *)previousEdge next:(Edge2D *)nextEdge {
    return [[[Edge2D alloc] initWithStart:startVertex end:endVertex previous:previousEdge next:nextEdge] autorelease];
}

- (id)initWithBoundary:(Line2D *)boundary outside:(Vector2f *)outside {
    return [self initWithBoundary:boundary previous:nil next:nil outside:outside];
}

- (id)initWithBoundary:(Line2D *)boundary previous:(Edge2D *)previousEdge outside:(Vector2f *)outside {
    return [self initWithBoundary:boundary previous:previousEdge next:nil outside:outside];
}

- (id)initWithBoundary:(Line2D *)boundary next:(Edge2D *)nextEdge outside:(Vector2f *)outside {
    return [self initWithBoundary:boundary previous:nil next:nextEdge outside:outside];
}

- (id)initWithBoundary:(Line2D *)boundary previous:(Edge2D *)previousEdge next:(Edge2D *)nextEdge outside:(Vector2f *)outside {
    if (!boundary)
        [NSException raise:NSInvalidArgumentException format:@"boundary must not be nil"];
    if (!outside)
        [NSException raise:NSInvalidArgumentException format:@"outside vector must not be nil"];

    if (self = [super init]) {
        boundaryLine = [[Line2D alloc] initWithLine:boundary];
        [self setPrevious:previousEdge];
        [self setNext:nextEdge];
        sVertex = nil;
        outsideVector = [[Vector2f alloc] initWithVector:outside];
    }
    
    return self;
}

- (id)initWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex {
    return [self initWithStart:startVertex end:endVertex previous:nil next:nil];
}

- (id)initWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex previous:(Edge2D *)previousEdge {
    return [self initWithStart:startVertex end:endVertex previous:previousEdge next:nil];
}

- (id)initWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex next:(Edge2D *)nextEdge {
    return [self initWithStart:startVertex end:endVertex previous:nil next:nextEdge];
}

- (id)initWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex previous:(Edge2D *)previousEdge next:(Edge2D *)nextEdge {
    if (!startVertex)
        [NSException raise:NSInvalidArgumentException format:@"start must not be nil"];
    if (!endVertex)
        [NSException raise:NSInvalidArgumentException format:@"end must not be nil"];
    if ([startVertex isEqual:endVertex])
        [NSException raise:NSInvalidArgumentException format:@"start and end must not be identical"];

    Line2D* boundary = [[Line2D alloc] initWithPoint1:startVertex point2:endVertex];
    Vector2f* outside = [[Vector2f alloc] initWithVector:endVertex];
    [outside sub:startVertex];
    [outside normalize];
    float x = [outside x];
    [outside setX:[outside y]];
    [outside setY:-x];
    
    self = [self initWithBoundary:boundary previous:previousEdge next:nextEdge outside:outside];
    [boundary release];
    [outside release];
    
    return self;
}


- (Vector2f *)startVertex {
    if (sVertex == nil && previous != nil) {
        Line2D* boundary = [previous boundary];
        sVertex = [boundary intersectWith:boundaryLine];
        [sVertex retain];
    }
    
    return sVertex;
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


- (Line2D *)boundary {
    return boundaryLine;
}

- (Vector2f *)outside {
    return outsideVector;
}

- (BOOL)isUpper {
    if (fpos([outsideVector y]))
        return YES;
    return fzero([outsideVector y]) && fneg([outsideVector x]);
}

- (BOOL)isLower {
    return ![self isUpper];
}

- (BOOL)contains:(Vector2f *)point {
    float y = [boundaryLine yAt:[point x]];
    if ([self isUpper])
        return flte([point y], y);

    return fgte([point y], y);
}

- (Vector2f *)intersectWith:(Edge2D *)edge {
    Vector2f* is = [boundaryLine intersectWith:[edge boundary]];
    if (is == nil)
        return nil;
    
    if (!finii([is x], [[self startVertex] x], [[self endVertex] x]) ||
        !finii([is x], [[edge startVertex] x], [[edge endVertex] x]))
        return nil;
    
    return is;
}

- (void)setPrevious:(Edge2D *)previousEdge {
    previous = previousEdge;
    [sVertex release];
    sVertex = nil;
}

- (void)setNext:(Edge2D *)nextEdge {
    [next release];
    next = [nextEdge retain];
}

- (void)open {
    [next setPrevious:nil];
    next = nil; // do not release
}

- (void)close:(Edge2D *)edge {
    [next release];
    next = edge; // do not retain
    [next setPrevious:self];
}

- (Edge2D *)previous {
    return previous;
}

- (Edge2D *)next {
    return next;
}

- (Edge2D *)appendEdgeWithBoundary:(Line2D *)boundary outside:(Vector2f *)outside {
    Edge2D* newEdge = [[Edge2D alloc] initWithBoundary:boundary previous:self next:next outside:outside];
    [next setPrevious:newEdge];
    [self setNext:newEdge];
    
    return [newEdge autorelease];
}

- (Edge2D *)appendEdgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex {
    Edge2D* newEdge = [[Edge2D alloc] initWithStart:startVertex end:endVertex previous:self next:next];
    [next setPrevious:newEdge];
    [self setNext:newEdge];
    
    return [newEdge autorelease];
}

- (Edge2D *)prependEdgeWithBoundary:(Line2D *)boundary outside:(Vector2f *)outside {
    Edge2D* newEdge = [[Edge2D alloc] initWithBoundary:boundary previous:previous next:self outside:outside];
    [previous setNext:newEdge];
    [self setPrevious:newEdge];
    
    return [newEdge autorelease];
}

- (Edge2D *)prependEdgeWithStart:(Vector2f *)startVertex end:(Vector2f *)endVertex {
    Edge2D* newEdge = [[Edge2D alloc] initWithStart:startVertex end:endVertex previous:previous next:self];
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
    [boundaryLine release];
    [sVertex release];
    [outsideVector release];
    [super dealloc];
}

@end

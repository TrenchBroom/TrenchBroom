//
//  Edge.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.04.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Edge.h"


@implementation Edge

- (id)initWithLine:(Line *)l {
    return [self initWithLine:l previous:nil next:nil];
}

- (id)initWithLine:(Line *)l previous:(Edge *)p {
    return [self initWithLine:l previous:p next:nil];
}

- (id)initWithLine:(Line *)l next:(Edge *)n {
    return [self initWithLine:l previous:nil next:n];
}

- (id)initWithLine:(Line *)l previous:(Edge *)p next:(Edge *)n {
    if (self = [super init]) {
        line = [l retain];
        [self setPrevious:p];
        [self setNext:n];
        startVertex = nil;
    }
    
    return self;
}

- (Vector3f *)startVertex {
    if (!startVertex && previous) {
        NSLog(@"%@", [previous class]);
        Line* l = [previous line];
        startVertex = [l intersectionWithLine:line];
        [startVertex retain];
    }

    return startVertex;
}

- (Vector3f *)endVertex {
    if (!next)
        return nil;
    
    return [next startVertex];
}

- (Line *)line {
    return line;
}

- (Edge *)insertAfter:(Line *)l until:(Edge *)e {
    if (!l)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
       
    Edge* newEdge = [[Edge alloc] initWithLine:l previous:self next:e];
    [e setPrevious:newEdge];
    [self setNext:newEdge];
    
    return [newEdge autorelease];
}

- (Edge *)insertBefore:(Line *)l {
    if (!l)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    Edge* newEdge = [[Edge alloc] initWithLine:l next:self];
    [self setPrevious:newEdge];
    
    return [newEdge autorelease];
}

- (Edge *)replaceWith:(Line *)l {
    if (!l)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];

    Edge* newEdge = [[Edge alloc] initWithLine:l previous:previous next:next];
    [previous setNext:newEdge];
    [next setPrevious:newEdge];
    [self setPrevious:nil];
    [self setNext:nil];
    
    return [newEdge autorelease];
}


- (void)setPrevious:(Edge *)p {
    previous = p;
    [startVertex release];
    startVertex = nil;
}

- (void)setNext:(Edge *)n {
    [next release];
    next = [n retain];
}

- (Edge *)previous {
    return previous;
}

- (Edge *)next {
    return next;
}

- (Vector3f *)intersectionWithLine:(Line *)l {
    if (!l)
        [NSException raise:NSInvalidArgumentException format:@"line must not be nil"];
    
    Vector3f* v = [line intersectionWithLine:l];
    if (!v)
        return nil;

    Vector3f* d = [line d];
    Vector3f* s = [self startVertex];
    Vector3f* e = [self endVertex];
    if (s) {
        Vector3f* t = [Vector3f sub:s subtrahend:v]; // we know that v and d have either the same or opposite directions
        if ([d x] > 0 && [t x] > 0 || 
            [d y] > 0 && [t y] > 0 || 
            [d z] > 0 && [t z] > 0)
            return nil;
    }
    
    if (e) {
        Vector3f* t = [Vector3f sub:e subtrahend:v];
        if ([d x] > 0 ^ [t x] > 0 || 
            [d y] > 0 ^ [t y] > 0 || 
            [d z] > 0 ^ [t z] > 0)
            return nil;
    }
    
    return v;
}


- (void)dealloc {
    [next release];
    [line release];
    [startVertex release];
    [super dealloc];
}

@end

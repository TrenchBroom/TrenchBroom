//
//  TrackingLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "TrackingLayer.h"
#import "ThickEdgeRenderer.h"
#import "VertexRenderer.h"
#import "Vertex.h"
#import "Brush.h"
#import "Face.h"
#import "Edge.h"
#import "Vertex.h"

@implementation TrackingLayer

- (id)init {
    if (self = [super init]) {
        edgeRenderer = [[ThickEdgeRenderer alloc] init];
        vertexRenderer = [[VertexRenderer alloc] init];
    }
    
    return self;
}

- (void)addBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self addFace:face];
}

- (void)removeBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self removeFace:face];
}

- (void)addFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    
    NSEnumerator* edgeEn = [[theFace edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject]))
        [self addEdge:edge];
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");

    NSEnumerator* edgeEn = [[theFace edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject]))
        [self removeEdge:edge];
}

- (void)addEdge:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    [edgeRenderer addEdge:theEdge];
    [self addVertex:[theEdge startVertex]];
    [self addVertex:[theEdge endVertex]];
}

- (void)removeEdge:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    [edgeRenderer removeEdge:theEdge];
    [self removeVertex:[theEdge startVertex]];
    [self removeVertex:[theEdge endVertex]];
}

- (void)addVertex:(Vertex *)theVertex{
    NSAssert(theVertex != nil, @"vertex must not be nil");
    [vertexRenderer addVertex:theVertex];
}

- (void)removeVertex:(Vertex *)theVertex {
    NSAssert(theVertex != nil, @"vertex must not be nil");
    [vertexRenderer removeVertex:theVertex];
}

- (void)render:(RenderContext *)renderContext {
    glColor4f(1, 1, 0, 0.5);
    glDisable(GL_DEPTH_TEST);
    [edgeRenderer render];
    [vertexRenderer render];
    glEnable(GL_DEPTH_TEST);
}

- (void)dealloc {
    [edgeRenderer release];
    [vertexRenderer release];
    [super dealloc];
}

@end

//
//  SelectionLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SelectionLayer.h"
#import <OpenGL/gl.h>
#import "MapWindowController.h"
#import "RenderContext.h"
#import "GridRenderer.h"
#import "VertexRenderer.h"
#import "Brush.h"
#import "Face.h"
#import "Edge.h"
#import "Vertex.h"
#import "Options.h"
#import "Grid.h"

@interface SelectionLayer (private)

- (void)optionsChanged:(NSNotification *)notification;

@end

@implementation SelectionLayer (private)

- (void)optionsChanged:(NSNotification *)notification {
    [gridRenderer setGridSize:[[options grid] size]];
}

@end

@implementation SelectionLayer

- (id)initWithWindowController:(MapWindowController *)theMapWindowController {
    if (self = [super initWithWindowController:theMapWindowController]) {
        options = [[theMapWindowController options] retain];
        gridRenderer = [[GridRenderer alloc] initWithGridSize:[[options grid] size]];
        vertexRenderer = [[VertexRenderer alloc] init];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(optionsChanged:) name:OptionsChanged object:options];
    }
    
    return self;
}

- (void)addBrushVertices:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* vertexEn = [[theBrush vertices] objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject]))
        [self addVertex:vertex];
}

- (void)removeBrushVertices:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* vertexEn = [[theBrush vertices] objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject]))
        [self removeVertex:vertex];
}

- (void)addFaceVertices:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    
    NSEnumerator* vertexEn = [[theFace vertices] objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject]))
        [self addVertex:vertex];
}

- (void)removeFaceVertices:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    
    NSEnumerator* vertexEn = [[theFace vertices] objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject]))
        [self removeVertex:vertex];
}

- (void)addEdgeVertices:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    [self addVertex:[theEdge startVertex]];
    [self addVertex:[theEdge endVertex]];
}

- (void)removeEdgeVertices:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    [self removeVertex:[theEdge startVertex]];
    [self removeVertex:[theEdge endVertex]];
}


- (void)addVertex:(Vertex *)theVertex {
    NSAssert(theVertex != nil, @"vertex must not be nil");
    [vertexRenderer addVertex:theVertex];
}

- (void)removeVertex:(Vertex *)theVertex {
    NSAssert(theVertex != nil, @"vertex must not be nil");
    [vertexRenderer removeVertex:theVertex];
}

- (void)addFace:(id <Face>)theFace {
    [super addFace:theFace];
    [gridRenderer addFace:theFace];
}

- (void)removeFace:(id <Face>)theFace {
    [super removeFace:theFace];
    [gridRenderer removeFace:theFace];
}

- (void)renderEdges {
    glColor4f(1, 0, 0, 1);
    glLineWidth(2);
    glDisable(GL_DEPTH_TEST);
    [edgeRenderer render];
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1);
}

- (void)renderGrid {
    glColor4f(1, 0, 0, 0.5f);
    [gridRenderer render];
}

- (void)renderVertices {
    glColor4f(1, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    [vertexRenderer render];
    glEnable(GL_DEPTH_TEST);
}

- (void)render:(RenderContext*)renderContext {
    [super render:renderContext];
    if ([[options grid] draw])
        [self renderGrid];
    [self renderVertices];
}

- (void)dealloc {
    [gridRenderer release];
    [vertexRenderer release];
    [options release];
    [super dealloc];
}

@end

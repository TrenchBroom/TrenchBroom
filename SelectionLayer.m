//
//  SelectionLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "SelectionLayer.h"
#import "GridRenderer.h"
#import "FaceHandleRenderer.h"

@implementation SelectionLayer

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager gridSize:(int)theGridSize drawGrid:(BOOL)doDrawGrid {
    if (self = [super initWithVbo:theVbo textureManager:theTextureManager]) {
        gridRenderer = [[GridRenderer alloc] initWithGridSize:theGridSize];
        drawGrid = doDrawGrid;
    }
    
    return self;
}

- (void)addFace:(id <Face>)theFace {
    [gridRenderer addFace:theFace];
    [super addFace:theFace];
}

- (void)removeFace:(id <Face>)theFace {
    [gridRenderer removeFace:theFace];
    [super removeFace:theFace];
}

- (void)preRenderEdges {
    glColor4f(1, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
}

- (void)postRenderEdges {
    glEnable(GL_DEPTH_TEST);
}

- (void)render:(RenderContext *)renderContext {
    [super render:renderContext];
    if (drawGrid) {
        glColor4f(1, 0, 0, 0.5f);
        [gridRenderer render];
    }
}

- (void)setGridSize:(int)theGridSize {
    [gridRenderer setGridSize:theGridSize];
}

- (void)setDrawGrid:(BOOL)doDrawGrid {
    drawGrid = doDrawGrid;
}

- (void)dealloc {
    [gridRenderer release];
    [super dealloc];
}

@end

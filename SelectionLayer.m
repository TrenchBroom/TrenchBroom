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
#import "Face.h"
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

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(optionsChanged:) name:OptionsChanged object:options];
    }
    
    return self;
}

- (void)addFace:(id <Face>)theFace includeEdges:(BOOL)includeEdges {
    [super addFace:theFace includeEdges:includeEdges];
    [gridRenderer addFace:theFace];
}

- (void)removeFace:(id <Face>)theFace includeEdges:(BOOL)includeEdges {
    [super removeFace:theFace includeEdges:includeEdges];
    [gridRenderer removeFace:theFace];
}

- (void)renderEdges {
    glColor4f(1, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    [edgeRenderer render];
    glEnable(GL_DEPTH_TEST);
}

- (void)render:(RenderContext*)renderContext {
    [super render:renderContext];
    
    if ([[options grid] draw]) {
        glColor4f(1, 0, 0, 0.5f);
        [gridRenderer render];
    }
}

- (void)dealloc {
    [gridRenderer release];
    [options release];
    [super dealloc];
}

@end

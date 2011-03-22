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
#import "LineRenderer.h"
#import "Figure.h"
#import "Face.h"
#import "Vector3f.h"
#import "Options.h"
#import "Grid.h"

@implementation SelectionLayer

- (id)initWithWindowController:(MapWindowController *)theMapWindowController {
    if (self = [super initWithWindowController:theMapWindowController]) {
        gridRenderer = [[LineRenderer alloc] init];
        options = [[theMapWindowController options] retain];
        gridFigures = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)render:(RenderContext *)renderContext {
    [super render:renderContext];
    glColor4f(1, 0, 0, 0.5f);
    [gridRenderer render];
}

- (void)renderEdges {
    glColor4f(1, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    [edgeRenderer render];
    glEnable(GL_DEPTH_TEST);
}

- (void)render:(RenderContext*)renderContext {
    [super render:renderContext];
    glColor4f(1, 0, 0, 0.5f);
    [gridRenderer render];
}

@end

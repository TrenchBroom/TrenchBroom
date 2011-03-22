//
//  GeometryLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GeometryLayer.h"
#import <OpenGL/gl.h>
#import "MapWindowController.h"
#import "MapDocument.h"
#import "GLResources.h"
#import "MapDocument.h"
#import "TextureManager.h"
#import "PolygonRenderer.h"
#import "LineRenderer.h"
#import "Face.h"
#import "Edge.h"
#import "Figure.h"
#import "PolygonFigure.h"
#import "LineFigure.h"
#import "RenderContext.h"
#import "Options.h"

@implementation GeometryLayer

- (id)initWithWindowController:(MapWindowController *)theMapWindowController {
    if (theMapWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        windowController = [theMapWindowController retain];
        
        MapDocument* map = [windowController document];
        GLResources* glResources = [map glResources];
        TextureManager* textureManager = [glResources textureManager];
        faceRenderer = [[PolygonRenderer alloc] initWithTextureManager:textureManager];
        edgeRenderer = [[LineRenderer alloc] init];
    }
    
    return self;
}

- (void)addFace:(id <Face>)theFace includeEdges:(BOOL)includeEdges {
    if (theFace == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];

    [faceRenderer addFigure:(id <PolygonFigure>)theFace];
    if (includeEdges) {
        NSEnumerator* edgeEn = [[theFace edges] objectEnumerator];
        Edge* edge;
        while ((edge = [edgeEn nextObject]))
            [edgeRenderer addFigure:(id <LineFigure>)edge];
    }
}

- (void)removeFace:(id <Face>)theFace includeEdges:(BOOL)includeEdges {
    if (theFace == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    
    [faceRenderer removeFigure:(id <PolygonFigure>)theFace];
    if (includeEdges) {
        NSEnumerator* edgeEn = [[theFace edges] objectEnumerator];
        Edge* edge;
        while ((edge = [edgeEn nextObject]))
            [edgeRenderer removeFigure:(id <LineFigure>)edge];
    }
}

- (void)addEdge:(Edge *)theEdge {
    if (theEdge == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge must not be nil"];
    
    [edgeRenderer addFigure:(id <LineFigure>)theEdge];
}

- (void)removeEdge:(Edge *)theEdge {
    if (theEdge == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge must not be nil"];
    
    [edgeRenderer removeFigure:(id <LineFigure>)theEdge];
}

- (void)renderTexturedFaces {
    [faceRenderer renderTextured:YES];
}

- (void)renderFlatFaces {
    [faceRenderer renderTextured:NO];
}

- (void)renderEdges {
    glColor4f(1, 1, 1, 0.5f);
    [edgeRenderer render];
}

- (void)render:(RenderContext *)renderContext {
    switch ([[renderContext options] renderMode]) {
        case RM_TEXTURED:
            [self renderTexturedFaces];
            glDisable(GL_TEXTURE_2D);
            [self renderEdges];
            break;
        case RM_FLAT:
            [self renderFlatFaces];
            [self renderEdges];
            break;
        case RM_WIREFRAME:
            glDisable(GL_TEXTURE_2D);
            [self renderEdges];
            break;
    }
}

- (void)invalidate {
    [faceRenderer invalidate];
    [edgeRenderer invalidate];
}

- (void)dealloc {
    [faceRenderer release];
    [edgeRenderer release];
    [windowController release];
    [super dealloc];
}

@end

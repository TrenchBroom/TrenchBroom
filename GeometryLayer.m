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
#import "FaceRenderer.h"
#import "ThinEdgeRenderer.h"
#import "Brush.h"
#import "Face.h"
#import "Edge.h"
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
        faceRenderer = [[FaceRenderer alloc] initWithTextureManager:textureManager];
        edgeRenderer = [[self createEdgeRenderer] retain];
    }
    
    return self;
}

- (id <EdgeRenderer>)createEdgeRenderer {
    return [[[ThinEdgeRenderer alloc] init] autorelease];
}

- (void)addBrushFaces:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self addFace:face];
}

- (void)removeBrushFaces:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self removeFace:face];
}

- (void)addBrushEdges:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* edgeEn = [[theBrush edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject]))
        [self addEdge:edge];
}

- (void)removeBrushEdges:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* edgeEn = [[theBrush edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject]))
        [self removeEdge:edge];
}

- (void)addFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    [faceRenderer addFace:theFace];
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    [faceRenderer removeFace:theFace];
}

- (void)addFaceEdges:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");

    NSEnumerator* edgeEn = [[theFace edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject]))
        [self addEdge:edge];
}

- (void)removeFaceEdges:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");

    NSEnumerator* edgeEn = [[theFace edges] objectEnumerator];
    Edge* edge;
    while ((edge = [edgeEn nextObject]))
        [self removeEdge:edge];
}

- (void)addEdge:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    [edgeRenderer addEdge:theEdge];
}

- (void)removeEdge:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    [edgeRenderer removeEdge:theEdge];
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

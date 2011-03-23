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
#import "EdgeRenderer.h"
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
        edgeRenderer = [[EdgeRenderer alloc] init];
    }
    
    return self;
}

- (void)addFace:(id <Face>)theFace includeEdges:(BOOL)includeEdges {
    NSAssert(theFace != nil, @"face must not be nil");

    [faceRenderer addFace:theFace];
    if (includeEdges) {
        NSEnumerator* edgeEn = [[theFace edges] objectEnumerator];
        Edge* edge;
        while ((edge = [edgeEn nextObject]))
            [edgeRenderer addEdge:edge];
    }
}

- (void)removeFace:(id <Face>)theFace includeEdges:(BOOL)includeEdges {
    NSAssert(theFace != nil, @"face must not be nil");
    
    [faceRenderer removeFace:theFace];
    if (includeEdges) {
        NSEnumerator* edgeEn = [[theFace edges] objectEnumerator];
        Edge* edge;
        while ((edge = [edgeEn nextObject]))
            [edgeRenderer removeEdge:edge];
    }
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

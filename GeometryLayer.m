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
#import "FaceFigure.h"
#import "EdgeFigure.h"

@implementation GeometryLayer

# pragma mark -
# pragma mark @implementation Layer

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

- (void)addFaceFigure:(FaceFigure *)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    [faceRenderer addFigure:theFigure];
}

- (void)removeFaceFigure:(FaceFigure *)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    [faceRenderer removeFigure:theFigure];
}

- (void)faceFigureChanged:(FaceFigure *)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    [theFigure invalidate];
    [faceRenderer invalidate];
}

- (void)addEdgeFigure:(EdgeFigure *)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    [edgeRenderer addFigure:theFigure];
}

- (void)removeEdgeFigure:(EdgeFigure *)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    [edgeRenderer removeFigure:theFigure];
}


# pragma mark -
# pragma mark GeometryLayer Implementation

- (id)initWithWindowController:(MapWindowController *)theMapWindowController {
    if (theMapWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        windowController = [theMapWindowController retain];
        
        MapDocument* map = [windowController document];
        GLResources* glResources = [map glResources];
        TextureManager* textureManager = [glResources textureManager];
        VBOBuffer* faceVbo = [glResources faceVbo];
        
        faceRenderer = [[PolygonRenderer alloc] initWithVbo:faceVbo textureManager:textureManager];
        edgeRenderer = [[LineRenderer alloc] init];
    }
    
    return self;
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

- (void)dealloc {
    [faceRenderer release];
    [edgeRenderer release];
    [windowController release];
    [super dealloc];
}

@end

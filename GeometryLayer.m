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
#import "Face.h"
#import "Figure.h"
#import "VBOBuffer.h"
#import "IntData.h"
#import "RenderContext.h"
#import "TextureManager.h"
#import "Texture.h"
#import "Options.h"

@implementation GeometryLayer

- (id)init {
    if (self = [super init]) {
        figures = [[NSMutableSet alloc] init];
        indexBuffers = [[NSMutableDictionary alloc] init];
        countBuffers = [[NSMutableDictionary alloc] init];
        buffersValid = NO;
    }
    
    return self;
}

- (id)initWithWindowController:(MapWindowController *)theMapWindowController {
    if (theMapWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        mapWindowController = [theMapWindowController retain];
    }
    
    return self;
}

- (void)addFigure:(id <Figure>)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    [figures addObject:theFigure];
    [self invalidate];
}

- (void)removeFigure:(id <Figure>)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    [figures removeObject:theFigure];
    [self invalidate];
}

- (void)prepare:(RenderContext *)renderContext {
    MapDocument* document = [mapWindowController document];
    GLResources* glResources = [document glResources];
    VBOBuffer* vbo = [glResources geometryVBO];
    
    [vbo mapBuffer];
    
    NSEnumerator* figureEn = [figures objectEnumerator];
    id <Figure> figure;
    while ((figure = [figureEn nextObject]))
        [figure prepare:renderContext];

    [vbo unmapBuffer];

    figureEn = [figures objectEnumerator];
    while ((figure = [figureEn nextObject])) {
        NSString* textureName = [figure texture];
        
        IntData* indexBuffer = [indexBuffers objectForKey:textureName];
        if (indexBuffer == nil) {
            indexBuffer = [[IntData alloc] init];
            [indexBuffers setObject:indexBuffer forKey:textureName];
            [indexBuffer release];
        }
        
        IntData* countBuffer = [countBuffers objectForKey:textureName];
        if (countBuffer == nil) {
            countBuffer = [[IntData alloc] init];
            [countBuffers setObject:countBuffer forKey:textureName];
            [countBuffer release];
        }
        
        [figure getIndex:indexBuffer count:countBuffer];
    }
    
    buffersValid = YES;
}

- (void)renderTextured:(RenderContext *)renderContext {
    TextureManager* textureManager = [renderContext textureManager];
    NSEnumerator* textureNameEn = [indexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        Texture* texture = [textureManager textureForName:textureName];
        [texture activate];
        
        IntData* indexBuffer = [indexBuffers objectForKey:textureName];
        IntData* countBuffer = [countBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glInterleavedArrays(GL_T2F_V3F, 0, NULL);
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);
    }
}

- (void)renderWireframe:(RenderContext *)renderContext {
    NSEnumerator* textureNameEn = [indexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        IntData* indexBuffer = [indexBuffers objectForKey:textureName];
        IntData* countBuffer = [countBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glVertexPointer(3, GL_FLOAT, 20, (const GLvoid *)8); // cast to pointer type to avoid compiler warning
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);
    }
}

- (void)renderFaces:(RenderContext *)renderContext {
    switch ([[renderContext options] renderMode]) {
        case RM_TEXTURED:
            if ([[renderContext options] isolationMode] == IM_NONE) {
                glEnable(GL_TEXTURE_2D);
                glPolygonMode(GL_FRONT, GL_FILL);
                glColor4f(0, 0, 0, 1);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                [self renderTextured:renderContext];
            }
            if ([[renderContext options] isolationMode] != IM_DISCARD) {
                glDisable(GL_TEXTURE_2D);
                glPolygonMode(GL_FRONT, GL_LINE);
                glColor4f(1, 1, 1, 0.5);
                [self renderWireframe:renderContext];
            }
            break;
        case RM_FLAT:
            break;
        case RM_WIREFRAME:
            if ([[renderContext options] isolationMode] != IM_DISCARD) {
                glDisable(GL_TEXTURE_2D);
                glPolygonMode(GL_FRONT, GL_LINE);
                glColor4f(1, 1, 1, 0.5);
                [self renderWireframe:renderContext];
            }
            break;
    }
}

- (void)render:(RenderContext *)renderContext {
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    
    MapDocument* document = [mapWindowController document];
    GLResources* glResources = [document glResources];
    VBOBuffer* vbo = [glResources geometryVBO];

    [vbo activate];
    if (!buffersValid)
        [self prepare:renderContext];

    [self renderFaces:renderContext];
    [vbo deactivate];
}

- (void)invalidate {
    if (buffersValid) {
        [indexBuffers removeAllObjects];
        [countBuffers removeAllObjects];
        buffersValid = NO;
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [indexBuffers release];
    [countBuffers release];
    [figures release];
    [mapWindowController release];
    [super dealloc];
}

@end

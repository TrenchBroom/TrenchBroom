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
#import "Map.h"
#import "Face.h"
#import "FaceFigure.h"
#import "VBOBuffer.h"
#import "IntData.h"
#import "RenderContext.h"
#import "TextureManager.h"
#import "Texture.h"
#import "Options.h"

@implementation GeometryLayer

- (id)init {
    if (self = [super init]) {
        faceFigures = [[NSMutableDictionary alloc] init];
        indexBuffers = [[NSMutableDictionary alloc] init];
        countBuffers = [[NSMutableDictionary alloc] init];
        buffersValid = NO;
    }
    
    return self;
}

- (void)faceChanged:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    Face* face = [userInfo objectForKey:FaceKey];
    
    FaceFigure* faceFigure = [faceFigures objectForKey:[face faceId]];
    if (faceFigure != nil) {
        [faceFigure invalidate];
        [indexBuffers removeAllObjects];
        [countBuffers removeAllObjects];
        buffersValid = NO;
    }
}

- (id)initWithWindowController:(MapWindowController *)theMapWindowController {
    if (theMapWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        mapWindowController = [theMapWindowController retain];
        
        MapDocument* document = [mapWindowController document];
        Map* map = [document map];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceFlagsChanged object:map];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceTextureChanged object:map];
        [center addObserver:self selector:@selector(faceChanged:) name:FaceGeometryChanged object:map];
    }
    
    return self;
}

- (void)addFigure:(id)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    FaceFigure* faceFigure = (FaceFigure *)theFigure;
    Face* face = [faceFigure face];
    [faceFigures setObject:faceFigure forKey:[face faceId]];
    
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    buffersValid = NO;
}

- (void)removeFigure:(id)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    FaceFigure* faceFigure = (FaceFigure *)theFigure;
    Face* face = [faceFigure face];
    [faceFigures removeObjectForKey:[face faceId]];
    
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    buffersValid = NO;
}

- (void)prepare:(RenderContext *)renderContext {
    MapDocument* document = [mapWindowController document];
    GLResources* glResources = [document glResources];
    VBOBuffer* vbo = [glResources geometryVBO];
    
    [vbo mapBuffer];
    
    NSEnumerator* figureEn = [faceFigures objectEnumerator];
    FaceFigure* faceFigure;
    while ((faceFigure = [figureEn nextObject]))
        [faceFigure prepare:renderContext];

    [vbo unmapBuffer];

    figureEn = [faceFigures objectEnumerator];
    while ((faceFigure = [figureEn nextObject])) {
        Face* face = [faceFigure face];
        NSString* textureName = [face texture];
        
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
        
        [faceFigure getIndex:indexBuffer count:countBuffer];
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
            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_FILL);
            glColor4f(0, 0, 0, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            [self renderTextured:renderContext];

            glDisable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_LINE);
            glColor4f(1, 1, 1, 0.5);
            [self renderWireframe:renderContext];
            break;
        case RM_FLAT:
            break;
        case RM_WIREFRAME:
            glDisable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_LINE);
            glColor4f(1, 1, 1, 0.5);
            [self renderWireframe:renderContext];
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

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [indexBuffers release];
    [countBuffers release];
    [faceFigures release];
    [mapWindowController release];
    [super dealloc];
}

@end

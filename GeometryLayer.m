//
//  GeometryLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GeometryLayer.h"
#import <OpenGL/gl.h>
#import "Face.h"
#import "FaceFigure.h"
#import "VBOBuffer.h"
#import "IntData.h"
#import "RenderContext.h"
#import "TextureManager.h"
#import "Texture.h"

@implementation GeometryLayer

- (id)init {
    if (self = [super init]) {
        faceFigures = [[NSMutableSet alloc] init];
        indexBuffers = [[NSMutableDictionary alloc] init];
        countBuffers = [[NSMutableDictionary alloc] init];
        buffersValid = NO;
    }
    
    return self;
}

- (id)initWithVbo:(VBOBuffer *)theVbo {
    if (theVbo == nil)
        [NSException raise:NSInvalidArgumentException format:@"vbo must not be nil"];
    
    if (self = [self init]) {
        vbo = [theVbo retain];
    }
    
    return self;
}

- (void)faceChanged:(NSNotification *)notification {
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    buffersValid = NO;
}

- (void)addFigure:(id)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    FaceFigure* faceFigure = (FaceFigure *)theFigure;
    [faceFigures addObject:faceFigure];
    
    Face* face = [faceFigure face];
    [face addObserver:self selector:@selector(faceChanged:) name:FaceFlagsChanged];
    [face addObserver:self selector:@selector(faceChanged:) name:FaceGeometryChanged];

    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    buffersValid = NO;
}

- (void)removeFigure:(id)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    FaceFigure* faceFigure = (FaceFigure *)theFigure;
    [faceFigures removeObject:faceFigure];

    Face* face = [faceFigure face];
    [face removeObserver:self];
    
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    buffersValid = NO;
}

- (void)prepare:(RenderContext *)renderContext {
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
    switch ([renderContext mode]) {
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
    
    [vbo activate];
    if (!buffersValid)
        [self prepare:renderContext];

    [self renderFaces:renderContext];
    [vbo deactivate];
}

- (void)dealloc {
    NSEnumerator* faceFigureEn = [faceFigures objectEnumerator];
    FaceFigure* faceFigure;
    while ((faceFigure = [faceFigureEn nextObject])) {
        Face* face = [faceFigure face];
        [face removeObserver:self];
    }
    
    [indexBuffers release];
    [countBuffers release];
    [faceFigures release];
    [vbo release];
    [super dealloc];
}

@end

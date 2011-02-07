//
//  GeometryLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GeometryLayer.h"
#import <OpenGL/gl.h>
#import "Brush.h"
#import "Face.h"
#import "BrushFigure.h"
#import "VBOBuffer.h"
#import "IntData.h"
#import "RenderContext.h"
#import "TextureManager.h"
#import "Texture.h"

@implementation GeometryLayer

- (id)init {
    if (self = [super init]) {
        faceFigures = [[NSMutableDictionary alloc] init]; // the smallest rendering thingy is a face!
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

- (void)brushChanged:(NSNotification *)notification {
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    buffersValid = NO;
}

- (void)addBrush:(Brush *)theBrush {
    if (theBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    BrushFigure* brushFigure = [[BrushFigure alloc] initWithBrush:theBrush vbo:vbo];
    [brushFigures setObject:brushFigure forKey:[theBrush brushId]];
    [brushFigure release];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(brushChanged:) name:BrushFlagsChanged object:theBrush];
    [center addObserver:self selector:@selector(brushChanged:) name:BrushGeometryChanged object:theBrush];
}

- (void)removeBrush:(Brush *)theBrush {
    if (theBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];

    [brushFigures removeObjectForKey:[theBrush brushId]];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:BrushFlagsChanged object:theBrush];
    [center removeObserver:self name:BrushGeometryChanged object:theBrush];
}

- (void)prepare:(RenderContext *)renderContext {
    [vbo mapBuffer];
    
    NSEnumerator* figureEn = [brushFigures objectEnumerator];
    BrushFigure* brushFigure;
    while ((brushFigure = [figureEn nextObject]))
        [brushFigure prepare:renderContext];

    [vbo unmapBuffer];

    while ((brushFigure = [figureEn nextObject])) {
        Brush* brush = [brushFigure brush];
        
        NSEnumerator* faceEn = [[brush faces] objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject])) {
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
            
            [brushFigure indexForFace:face indexBuffer:indexBuffer countBuffer:countBuffer];
        }
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
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [indexBuffers release];
    [countBuffers release];
    [brushFigures release];
    [vbo release];
    [super dealloc];
}

@end

//
//  PolygonRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "PolygonRenderer.h"
#import "PolygonFigure.h"
#import "TextureManager.h"
#import "Texture.h"
#import "VBOBuffer.h"
#import "IntData.h"
#import "FigureFilter.h"

@interface PolygonRenderer (private)

- (void)prepare;

@end

@implementation PolygonRenderer (private)

- (void)prepare {
    [vbo mapBuffer];

    NSEnumerator* figureEn = [figures objectEnumerator];
    id <PolygonFigure> figure;
    while ((figure = [figureEn nextObject]))
        [figure prepareWithVbo:vbo textureManager:textureManager];
    
    [vbo unmapBuffer];
    
    figureEn = [figures objectEnumerator];
    while ((figure = [figureEn nextObject])) {
        if (filter == nil || [filter passes:figure]) {
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
    }
    
    valid = YES;
}

@end

@implementation PolygonRenderer

- (id)init {
    if (self = [super init]) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        figures = [[NSMutableSet alloc] init];
        indexBuffers = [[NSMutableDictionary alloc] init];
        countBuffers = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithTextureManager:(TextureManager *)theTextureManager {
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    
    if (self = [self init]) {
        textureManager = [theTextureManager retain];
    }
    
    return self;
}

- (void)addFigure:(id <PolygonFigure>)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    NSAssert(![figures containsObject:theFigure], @"figure already belongs to this renderer");
    [figures addObject:theFigure];
    [self invalidate];
}

- (void)removeFigure:(id <PolygonFigure>)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    NSAssert([figures containsObject:theFigure], @"figure does not belong to this renderer");
    [figures removeObject:theFigure];
    [self invalidate];
}

- (void)setFilter:(id <FigureFilter>)theFilter {
    if (filter == theFilter)
        return;
    
    [filter release];
    filter = [theFilter retain];
    [self invalidate];
}

- (void)renderTextured:(BOOL)textured {
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    glShadeModel(GL_FLAT);
    
    [vbo activate];
    
    if (!valid)
        [self prepare];
    
    glPolygonMode(GL_FRONT, GL_FILL);
    if (textured) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    NSEnumerator* textureNameEn = [indexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        if (textured) {
            Texture* texture = [textureManager textureForName:textureName];
            [texture activate];
        }
        
        IntData* indexBuffer = [indexBuffers objectForKey:textureName];
        IntData* countBuffer = [countBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glInterleavedArrays(GL_T2F_C3F_V3F, 0, NULL);
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);
    }
    
    // not sure why this is neccessary
    glDisableClientState(GL_COLOR_ARRAY);

    [vbo deactivate];
    glDisable(GL_POLYGON_OFFSET_FILL);
}

- (void)invalidate {
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    valid = NO;
}

- (void)dealloc {
    [filter release];
    [figures release];
    [indexBuffers release];
    [countBuffers release];
    [vbo release];
    [textureManager release];
    [super dealloc];
}

@end

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
#import "Edge.h"
#import "Figure.h"
#import "VBOBuffer.h"
#import "IntData.h"
#import "RenderContext.h"
#import "TextureManager.h"
#import "Texture.h"
#import "Options.h"
#import "IdGenerator.h"

static NSString* const FaceVboKey = @"FaceVbo";

@implementation GeometryLayer

- (id)init {
    if (self = [super init]) {
        faceFigures = [[NSMutableSet alloc] init];
        edgeFigures = [[NSMutableSet alloc] init];
        faceIndexBuffers = [[NSMutableDictionary alloc] init];
        faceCountBuffers = [[NSMutableDictionary alloc] init];
        edgeVboKey = [[IdGenerator sharedGenerator] getId];
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
    
    id object = [theFigure object];
    if ([object conformsToProtocol:@protocol(Face)]) {
        [faceFigures addObject:theFigure];
    } else if ([object isKindOfClass:[Edge class]]) {
        [edgeFigures addObject:theFigure];
        [theFigure invalidate];
    }
    
    [self invalidate];
}

- (void)removeFigure:(id <Figure>)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];

    id object = [theFigure object];
    if ([object conformsToProtocol:@protocol(Face)]) {
        [faceFigures removeObject:theFigure];
    } else if ([object isKindOfClass:[Edge class]]) {
        [edgeFigures removeObject:theFigure];
        [theFigure invalidate];
    }
    
    [self invalidate];
}

- (void)prepare {
    MapDocument* document = [mapWindowController document];
    GLResources* glResources = [document glResources];
    VBOBuffer* faceVbo = [glResources vboForKey:FaceVboKey];
    VBOBuffer* edgeVbo = [glResources vboForKey:edgeVboKey];
    
    [faceVbo activate];
    [faceVbo mapBuffer];
    
    NSEnumerator* faceFigureEn = [faceFigures objectEnumerator];
    id <Figure> faceFigure;
    while ((faceFigure = [faceFigureEn nextObject]))
        [faceFigure prepareWithVbo:faceVbo textureManager:[glResources textureManager]];

    [faceVbo unmapBuffer];
    [faceVbo deactivate];

    faceFigureEn = [faceFigures objectEnumerator];
    while ((faceFigure = [faceFigureEn nextObject])) {
        id <Face> face = [faceFigure object];
        NSString* textureName = [face texture];
        
        IntData* indexBuffer = [faceIndexBuffers objectForKey:textureName];
        if (indexBuffer == nil) {
            indexBuffer = [[IntData alloc] init];
            [faceIndexBuffers setObject:indexBuffer forKey:textureName];
            [indexBuffer release];
        }
        
        IntData* countBuffer = [faceCountBuffers objectForKey:textureName];
        if (countBuffer == nil) {
            countBuffer = [[IntData alloc] init];
            [faceCountBuffers setObject:countBuffer forKey:textureName];
            [countBuffer release];
        }
        
        [faceFigure getIndex:indexBuffer count:countBuffer];
    }
    
    [edgeVbo activate];
    [edgeVbo mapBuffer];
    
    NSEnumerator* edgeFigureEn = [edgeFigures objectEnumerator];
    id <Figure> edgeFigure;
    while ((edgeFigure = [edgeFigureEn nextObject]))
        [edgeFigure prepareWithVbo:edgeVbo textureManager:[glResources textureManager]];

    [edgeVbo pack];
    [edgeVbo unmapBuffer];
    [edgeVbo deactivate];

    buffersValid = YES;
}

- (void)renderFaces {
    MapDocument* document = [mapWindowController document];
    GLResources* glResources = [document glResources];
    VBOBuffer* faceVbo = [glResources vboForKey:FaceVboKey];

    [faceVbo activate];
    
    TextureManager* textureManager = [glResources textureManager];
    NSEnumerator* textureNameEn = [faceIndexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        Texture* texture = [textureManager textureForName:textureName];
        [texture activate];
        
        IntData* indexBuffer = [faceIndexBuffers objectForKey:textureName];
        IntData* countBuffer = [faceCountBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glInterleavedArrays(GL_T2F_V3F, 0, NULL);
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);
    }
    
    [faceVbo deactivate];
}

- (void)renderEdges {
    MapDocument* document = [mapWindowController document];
    GLResources* glResources = [document glResources];
    VBOBuffer* edgeVbo = [glResources vboForKey:edgeVboKey];

    [edgeVbo activate];
    
    glVertexPointer(3, GL_FLOAT, 0, NULL);
    glDrawArrays(GL_LINES, 0, [edgeFigures count] * 2);
    
    [edgeVbo deactivate];
    
}

- (void)doRender:(RenderContext *)renderContext {
    switch ([[renderContext options] renderMode]) {
        case RM_TEXTURED:
            if ([[renderContext options] isolationMode] == IM_NONE) {
                glEnable(GL_TEXTURE_2D);
                glPolygonMode(GL_FRONT, GL_FILL);
                glColor4f(0, 0, 0, 1);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                [self renderFaces];
            }
        case RM_WIREFRAME:
            if ([[renderContext options] isolationMode] != IM_DISCARD) {
                glDisable(GL_TEXTURE_2D);
                glColor4f(1, 1, 1, 0.5);
                [self renderEdges];
            }
            break;
        case RM_FLAT:
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

    if (!buffersValid)
        [self prepare];

    [self doRender:renderContext];
}

- (void)invalidate {
    if (buffersValid) {
        [faceIndexBuffers removeAllObjects];
        [faceCountBuffers removeAllObjects];
        buffersValid = NO;
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [faceIndexBuffers release];
    [faceCountBuffers release];
    [faceFigures release];
    [edgeFigures release];
    [mapWindowController release];
    [super dealloc];
}

@end

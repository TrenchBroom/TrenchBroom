//
//  GeometryLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GeometryLayer.h"
#import <OpenGL/gl.h>
#import "TextureManager.h"
#import "Brush.h"
#import "Face.h"
#import "RenderContext.h"
#import "Options.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "IntData.h"

@implementation GeometryLayer

- (id)init {
    if (self = [super init]) {
        faces = [[NSMutableSet alloc] init];
        addedFaces = [[NSMutableSet alloc] init];
        removedFaces = [[NSMutableSet alloc] init];
        indexBuffers = [[NSMutableDictionary alloc] init];
        countBuffers = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager {
    NSAssert(theVbo != nil, @"VBO must not be nil");
    NSAssert(theTextureManager != nil, @"texture manager must not be nil");
    
    if (self = [self init]) {
        sharedVbo = [theVbo retain];
        textureManager = [theTextureManager retain];
    }
    
    return self;
}

- (void)addBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self addFace:face];
}

- (void)removeBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    
    NSEnumerator* faceEn = [[theBrush faces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [self removeFace:face];
}

- (void)addFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    [addedFaces addObject:theFace];
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    if ([addedFaces containsObject:theFace])
        [addedFaces removeObject:theFace];
    else
        [removedFaces addObject:theFace];
}

- (void)renderFaces:(BOOL)textured {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
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
    glDisable(GL_POLYGON_OFFSET_FILL);
}

- (void)preRenderEdges {
    glColor4f(1, 1, 1, 1);
}

- (void)postRenderEdges {
}

- (void)renderEdges {
    [self preRenderEdges];
    glDisable(GL_TEXTURE_2D);
    NSEnumerator* textureNameEn = [indexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        IntData* indexBuffer = [indexBuffers objectForKey:textureName];
        IntData* countBuffer = [countBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glVertexPointer(3, GL_FLOAT, 8 * sizeof(float), 5 * sizeof(float));
        glMultiDrawArrays(GL_LINE_LOOP, indexBytes, countBytes, primCount);
    }
    [self postRenderEdges];
}

- (void)render:(RenderContext *)renderContext {
    [self validate];
    [sharedVbo activate];
    switch ([[renderContext options] renderMode]) {
        case RM_TEXTURED:
            [self renderFaces:YES];
            [self renderEdges];
            break;
        case RM_FLAT:
            [self renderFaces:NO];
            [self renderEdges];
            break;
        case RM_WIREFRAME:
            [self renderEdges];
            break;
    }
    [sharedVbo deactivate];
}

- (void)validateFaces:(NSSet *)invalidFaces {
    NSEnumerator* faceEn = [invalidFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        VBOMemBlock* block = [face memBlock];
        NSAssert(block != nil, @"VBO mem block must be in shared block map");
        NSAssert([block state] == BS_USED_VALID, @"VBO mem block must be valid");
        
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
        
        int index = [block address] / (8 * sizeof(float));
        int count = [[face vertices] count];
        [indexBuffer appendInt:index];
        [countBuffer appendInt:count];
    }
}

- (void)validate {
    BOOL valid = YES;
    if ([removedFaces count] > 0) {
        int c = [faces count];
        [faces minusSet:removedFaces];
        valid = c == [faces count];
        [removedFaces removeAllObjects];
    }
    
    if ([addedFaces count] > 0) {
        [addedFaces minusSet:faces]; // to be safe
        if (valid)
            [self validateFaces:addedFaces];
        [faces unionSet:addedFaces];
        [addedFaces removeAllObjects];
    }
    
    if (!valid) {
        [indexBuffers removeAllObjects];
        [countBuffers removeAllObjects];
        [self validateFaces:faces];
    }
}


- (void)dealloc {
    [sharedVbo release];
    [faces release];
    [addedFaces release];
    [removedFaces release];
    [indexBuffers release];
    [countBuffers release];
    [textureManager release];
    [super dealloc];
}

@end

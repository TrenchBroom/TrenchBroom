//
//  PolygonRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FaceRenderer.h"
#import "Brush.h"
#import "Face.h"
#import "Vertex.h"
#import "TextureManager.h"
#import "Texture.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "IntData.h"
#import "RenderFilter.h"
#import "MathCache.h"
#import "Vector2f.h"
#import "Vector3f.h"

@interface FaceRenderer (private)

- (void)writeFace:(id <Face>)theFace indexBuffer:(IntData *)indexBuffer countBuffer:(IntData *)countBuffer;
- (void)validate;

@end

@implementation FaceRenderer (private)

- (void)writeFace:(id <Face>)theFace indexBuffer:(IntData *)indexBuffer countBuffer:(IntData *)countBuffer {
    MathCache* cache = [MathCache sharedCache];
    Vector3f* color = [cache vector3f];
    id <Brush> brush = [theFace brush];
    [color setX:[brush flatColor][0]];
    [color setY:[brush flatColor][1]];
    [color setZ:[brush flatColor][2]];

    Vector2f* texCoords = [cache vector2f];
    Texture* texture = [textureManager textureForName:[theFace texture]];
    int width = texture != nil ? [texture width] : 1;
    int height = texture != nil ? [texture height] : 1;
    
    int vertexSize = 8 * sizeof(float);
    NSArray* vertices = [theFace vertices];
    
    VBOMemBlock* block = [vbo allocMemBlock:vertexSize * [vertices count]];
    [indexBuffer appendInt:[block address] / vertexSize];
    [countBuffer appendInt:[vertices count]];
    
    int offset = 0;
    NSEnumerator* vertexEn = [vertices objectEnumerator];
    Vertex* vertex;
    while ((vertex = [vertexEn nextObject])) {
        [theFace texCoords:texCoords forVertex:[vertex vector]];
        [texCoords setX:[texCoords x] / width];
        [texCoords setY:[texCoords y] / height];
        offset = [block writeVector2f:texCoords offset:offset];
        offset = [block writeVector3f:color offset:offset];
        offset = [block writeVector3f:[vertex vector] offset:offset];
    }
    
    [cache returnVector3f:color];
    [cache returnVector2f:texCoords];
    [block setState:BS_USED_VALID];
}

- (void)validate {
    if (!valid) {
        [vbo deactivate];
        [vbo freeAllBlocks];
        
        [vbo activate];
        [vbo mapBuffer];
        
        
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            if (filter == nil || [filter facePasses:face]) {
                NSString* texture = [face texture];
                
                IntData* indexBuffer = [indexBuffers objectForKey:texture];
                if (indexBuffer == nil) {
                    indexBuffer = [[IntData alloc] init];
                    [indexBuffers setObject:indexBuffer forKey:texture];
                    [indexBuffer release];
                }
                
                IntData* countBuffer = [countBuffers objectForKey:texture];
                if (countBuffer == nil) {
                    countBuffer = [[IntData alloc] init];
                    [countBuffers setObject:countBuffer forKey:texture];
                    [countBuffer release];
                }
                
                [self writeFace:face indexBuffer:indexBuffer countBuffer:countBuffer];
            }
        }
        [vbo unmapBuffer];
        
        valid = YES;
    }
}

@end

@implementation FaceRenderer

- (id)init {
    if (self = [super init]) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        faces = [[NSMutableSet alloc] init];
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

- (void)addFace:(id <Face>)theFace; {
    NSAssert(theFace != nil, @"face must not be nil");
    NSAssert(![faces containsObject:theFace], @"face is already handled by this renderer");
    [faces addObject:theFace];
    [self invalidate];
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    NSAssert([faces containsObject:theFace], @"face is not handled by this renderer");
    [faces removeObject:theFace];
    [self invalidate];
}

- (void)setFilter:(id <RenderFilter>)theFilter {
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
    [self validate];
    
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

    [vbo deactivate];
}

- (void)invalidate {
    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    valid = NO;
}

- (void)dealloc {
    [filter release];
    [faces release];
    [indexBuffers release];
    [countBuffers release];
    [vbo release];
    [textureManager release];
    [super dealloc];
}

@end

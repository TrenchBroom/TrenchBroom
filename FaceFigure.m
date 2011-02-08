//
//  FaceFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FaceFigure.h"
#import "Brush.h"
#import "Face.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Vector3f.h"
#import "Vector2f.h"
#import "TextureManager.h"
#import "Texture.h"
#import "IntData.h"
#import "RenderContext.h"


@implementation FaceFigure

- (id)initWithFace:(Face *)theFace vbo:(VBOBuffer *)theVbo {
    if (theFace == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    if (theVbo == nil)
        [NSException raise:NSInvalidArgumentException format:@"VBO buffer must not be nil"];
    
    if (self = [self init]) {
        face = [theFace retain];
        vbo = [theVbo retain];
        
        Brush* brush = [face brush];
        int vertexCount = [[brush verticesForFace:face] count];
        block = [[vbo allocMemBlock:5 * sizeof(float) * vertexCount] retain];

        [face addObserver:self selector:@selector(faceChanged:) name:FaceGeometryChanged];
        [face addObserver:self selector:@selector(faceChanged:) name:FaceFlagsChanged];
    }
    
    return self;
}

- (Face *)face {
    return face;
}

- (void)faceChanged:(NSNotification *)notification {
    [vbo freeMemBlock:block];
    [block release];

    Brush* brush = [face brush];
    int vertexCount = [[brush verticesForFace:face] count];
    block = [[vbo allocMemBlock:5 * sizeof(float) * vertexCount] retain];
}

- (void)prepare:(RenderContext *)renderContext {
    if ([block state] == BS_USED_INVALID) {
        Brush* brush = [face brush];
        TextureManager* textureManager = [renderContext textureManager];
        Vector2f* texCoords = [[Vector2f alloc] init];
        
        NSString* textureName = [face texture];
        Texture* texture = [textureManager textureForName:textureName];
        int width = texture != nil ? [texture width] : 1;
        int height = texture != nil ? [texture height] : 1;
        
        int vertexSize = 5 * sizeof(float);
        NSArray* vertices = [brush verticesForFace:face];
        vboIndex = [block address] / vertexSize;
        vboCount = [vertices count];
        
        int offset = 0;
        NSEnumerator* vertexEn = [vertices objectEnumerator];
        Vector3f* vertex;
        while ((vertex = [vertexEn nextObject])) {
            [face texCoords:texCoords forVertex:vertex];
            [texCoords setX:[texCoords x] / width];
            [texCoords setY:[texCoords y] / height];
            offset = [block writeVector2f:texCoords offset:offset];
            offset = [block writeVector3f:vertex offset:offset];
        }
        
        [texCoords release];
        [block setState:BS_USED_VALID];
    }
}

- (void)getIndex:(IntData *)theIndexBuffer count:(IntData *)theCountBuffer {
    if ([block state] != BS_USED_VALID)
        [NSException raise:@"InvalidMemBlockState" format:@"VBO memory block is not valid"];
        
    [theIndexBuffer appendInt:vboIndex];
    [theCountBuffer appendInt:vboCount];
}

- (void)dealloc {
    [face removeObserver:self];
    [face release];
    [vbo freeMemBlock:block];
    [block release];
    [vbo release];
    [super dealloc];
}
@end

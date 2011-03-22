//
//  FaceFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 22.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FaceFigure.h"
#import "Face.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "IntData.h"
#import "TextureManager.h"
#import "Texture.h"
#import "MathCache.h"
#import "Vector2f.h"
#import "Vector3f.h"

@implementation FaceFigure

- (id)initWithFace:(id <Face>)theFace textureManager:(TextureManager *)theTextureManager {
    NSAssert(theFace != nil, @"face must not be nil");
    NSAssert(theTextureManager != nil, @"texture manager must not be nil");
    
    if (self = [self init]) {
        face = [theFace retain];
        textureManager = [theTextureManager retain];
    }
    
    return self;
}

- (void)dealloc {
    [self invalidate];
    [face release];
    [textureManager release];
    [super dealloc];
}

# pragma mark -
# pragma mark @implementation Figure

- (void)updateVBO:(VBOBuffer *)theVbo {
    if (block != nil && [block vbo] == theVbo && [block state] == BS_USED_VALID)
        return;
    
    if (block != nil && [block vbo] != theVbo)
        [self invalidate];
    
    if (block == nil) {
        int vertexCount = [[face vertices] count];
        block = [[theVbo allocMemBlock:8 * sizeof(float) * vertexCount] retain];
    }
    
    if ([block state] == BS_USED_INVALID) {
        MathCache* cache = [MathCache sharedCache];
        Vector3f* color = [cache vector3f];
        [color setX:[[face brush] flatColor][0]];
        [color setY:[[face brush] flatColor][1]];
        [color setZ:[[face brush] flatColor][2]];
        
        Vector2f* texCoords = [cache vector2f];
        Texture* tex = [textureManager textureForName:[face texture]];
        int width = tex != nil ? [tex width] : 1;
        int height = tex != nil ? [tex height] : 1;
        
        int vertexSize = 8 * sizeof(float);
        NSArray* vertices = [face vertices];
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
            offset = [block writeVector3f:color offset:offset];
            offset = [block writeVector3f:vertex offset:offset];
        }
        
        [cache returnVector3f:color];
        [cache returnVector2f:texCoords];
        [block setState:BS_USED_VALID];
    }
}

# pragma mark -
# pragma mark @implementation PolygonFigure

- (void)invalidate {
    [block free];
    [block release];
    block = nil;
}

- (NSString *)texture {
    return [face texture];
}

- (void)getIndex:(IntData *)theIndexBuffer count:(IntData *)theCountBuffer {
    if ([block state] != BS_USED_VALID)
        [NSException raise:@"InvalidMemBlockState" format:@"VBO memory block is not valid"];
    
    [theIndexBuffer appendInt:vboIndex];
    [theCountBuffer appendInt:vboCount];
}

@end

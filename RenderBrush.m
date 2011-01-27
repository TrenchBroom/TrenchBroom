//
//  RenderBrush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "RenderBrush.h"
#import "Brush.h"
#import "Face.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Vector3f.h"
#import "Vector2f.h"
#import "VBOArrayEntry.h"
#import "TextureManager.h"
#import "Texture.h"

@implementation RenderBrush

- (id)init {
    if (self = [super init]) {
        arrayInfoForTexture = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithBrush:(Brush *)theBrush vboBuffer:(VBOBuffer *)theVboBuffer {
    if (theBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    if (theVboBuffer == nil)
        [NSException raise:NSInvalidArgumentException format:@"VBO buffer must not be nil"];
    
    if (self = [self init]) {
        brush = [theBrush retain];
        vboBuffer = [theVboBuffer retain];
        
        int vertexCount = 0;
        NSEnumerator* faceEn = [[brush faces] objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject]))
            vertexCount += [[brush verticesForFace:face] count];
        
        vboMemBlock = [[vboBuffer allocMemBlock:5 * sizeof(float) * vertexCount] retain];
    }
    
    return self;
}

- (Brush *)brush {
    return brush;
}

- (void)renderWithContext:(id <RenderContext>)renderContext {
    [renderContext renderBrush:self];
}

- (NSDictionary *)prepareWithTextureManager:(TextureManager *)theTextureManager {
    if ([vboMemBlock state] == BS_USED_INVALID) {
        int offset = 0;
        int vertexSize = 5 * sizeof(float);
        Vector2f* texCoords = [[Vector2f alloc] init];

        NSEnumerator* faceEn = [[brush faces] objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject])) {
            NSString* textureName = [face texture];
            Texture* texture = [theTextureManager textureForName:textureName];
            int width = texture != nil ? [texture width] : 1;
            int height = texture != nil ? [texture height] : 1;
            
            NSMutableArray* entries = [arrayInfoForTexture objectForKey:textureName];
            if (entries == nil) {
                entries = [[NSMutableArray alloc] init];
                [arrayInfoForTexture setObject:entries forKey:textureName];
                [entries release];
            }
            
            NSArray* vertices = [brush verticesForFace:face];
            int count = [vertices count];
            int index = ([vboMemBlock address] + offset) / vertexSize;
            
            VBOArrayEntry* entry = [[VBOArrayEntry alloc] initWithIndex:index count:count];
            [entries addObject:entry];
            [entry release];
            
            NSEnumerator* vertexEn = [vertices objectEnumerator];
            Vector3f* vertex;
            while ((vertex = [vertexEn nextObject])) {
                [face texCoords:texCoords forVertex:vertex];
                [texCoords setX:[texCoords x] / width];
                [texCoords setY:[texCoords y] / height];
                offset = [vboMemBlock writeVector2f:texCoords offset:offset];
                offset = [vboMemBlock writeVector3f:vertex offset:offset];
            }
        }
        
        [texCoords release];
        [vboMemBlock setState:BS_USED_VALID];
    }
    
    return arrayInfoForTexture;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [arrayInfoForTexture release];
    [vboMemBlock release];
    [vboBuffer release];
    [brush release];
    [super dealloc];
}

@end

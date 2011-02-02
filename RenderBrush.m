//
//  RenderBrush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "RenderBrush.h"
#import "RenderEntity.h"
#import "Brush.h"
#import "Face.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Vector3f.h"
#import "Vector2f.h"
#import "VBOArrayEntry.h"
#import "TextureManager.h"
#import "Texture.h"
#import "IntData.h"

@implementation RenderBrush

- (id)init {
    if (self = [super init]) {
        faceEntries = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)brushChanged:(NSNotification *)notification {
    [faceBlock setState:BS_USED_INVALID];
    [faceEntries removeAllObjects];
    [renderEntity brushChanged];
}

- (id)initInEntity:(RenderEntity *)theRenderEntity withBrush:(Brush *)theBrush faceVBO:(VBOBuffer *)theFaceVBO {
    if (theRenderEntity == nil)
        [NSException raise:NSInvalidArgumentException format:@"render entity must not be nil"];
    if (theBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    if (theFaceVBO == nil)
        [NSException raise:NSInvalidArgumentException format:@"face VBO buffer must not be nil"];
    
    if (self = [self init]) {
        renderEntity = [theRenderEntity retain];
        brush = [theBrush retain];
        faceVBO = [theFaceVBO retain];
        
        int vertexCount = 0;
        NSEnumerator* faceEn = [[brush faces] objectEnumerator];
        Face* face;
        while ((face = [faceEn nextObject]))
            vertexCount += [[brush verticesForFace:face] count];
        
        faceBlock = [[faceVBO allocMemBlock:5 * sizeof(float) * vertexCount] retain];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(brushChanged:) name:BrushGeometryChanged object:brush];
        [center addObserver:self selector:@selector(brushChanged:) name:BrushFlagsChanged object:brush];
    }
    
    return self;
}

- (Brush *)brush {
    return brush;
}

- (void)prepareFacesWithTextureManager:(TextureManager *)theTextureManager {
    if ([faceBlock state] == BS_USED_INVALID) {
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
            
            NSArray* vertices = [brush verticesForFace:face];
            int index = ([faceBlock address] + offset) / vertexSize;

            VBOArrayEntry* entry = [[VBOArrayEntry alloc] initWithIndex:index count:[vertices count]];
            [faceEntries setObject:entry forKey:[face faceId]];
            [entry release];
            
            NSEnumerator* vertexEn = [vertices objectEnumerator];
            Vector3f* vertex;
            while ((vertex = [vertexEn nextObject])) {
                [face texCoords:texCoords forVertex:vertex];
                [texCoords setX:[texCoords x] / width];
                [texCoords setY:[texCoords y] / height];
                offset = [faceBlock writeVector2f:texCoords offset:offset];
                offset = [faceBlock writeVector3f:vertex offset:offset];
            }
        }
        
        [texCoords release];
        [faceBlock setState:BS_USED_VALID];
    }
}

- (void)indexForFace:(Face *)face indexBuffer:(IntData *)theIndexBuffer countBuffer:(IntData *)theCountBuffer {
    VBOArrayEntry* entry = [faceEntries objectForKey:[face faceId]];
    if (entry != nil) {
        [theIndexBuffer appendInt:[entry index]];
        [theCountBuffer appendInt:[entry count]];
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [faceEntries release];
    [faceBlock release];
    [faceVBO release];
    [brush release];
    [renderEntity release];
    [super dealloc];
}

@end

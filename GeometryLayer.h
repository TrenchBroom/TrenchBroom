//
//  GeometryLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@class RenderContext;
@class VBOBuffer;
@class TextureManager;

@interface GeometryLayer : NSObject <Layer> {
    VBOBuffer* sharedVbo;
    NSMutableSet* faces;
    NSMutableSet* addedFaces;
    NSMutableSet* removedFaces;
    NSMutableDictionary* indexBuffers;
    NSMutableDictionary* countBuffers;
    TextureManager* textureManager;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager;


- (void)renderFaces:(BOOL)textured;
- (void)renderEdges;
- (void)preRenderEdges;
- (void)postRenderEdges;

- (void)render:(RenderContext *)renderContext;

@end

//
//  GeometryBrushLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BrushLayer.h"

@class RenderContext;
@class VBOBuffer;
@class TextureManager;
@class Grid;

@interface GeometryLayer : NSObject <BrushLayer> {
    VBOBuffer* sharedVbo;
    NSMutableSet* faces;
    NSMutableSet* addedFaces;
    NSMutableSet* removedFaces;
    NSMutableDictionary* indexBuffers;
    NSMutableDictionary* countBuffers;
    TextureManager* textureManager;
    Grid* grid;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager grid:(Grid *)theGrid;;


- (void)renderFaces:(BOOL)textured;
- (void)renderEdges;
- (void)preRenderEdges;
- (void)postRenderEdges;

- (void)render:(RenderContext *)renderContext;

- (void)validateFaces:(NSSet *)invalidFaces;
- (void)validate;

@end

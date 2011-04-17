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
@class Options;
@protocol Filter;

@interface GeometryLayer : NSObject <BrushLayer> {
    VBOBuffer* sharedVbo;
    NSMutableSet* faces;
    NSMutableSet* addedFaces;
    NSMutableSet* removedFaces;
    NSMutableDictionary* indexBuffers;
    NSMutableDictionary* countBuffers;
    TextureManager* textureManager;
    Options* options;
    id <Filter> filter;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager options:(Options *)theOptions;


- (void)renderFaces:(BOOL)textured;
- (void)renderEdges;
- (void)preRenderEdges;
- (void)postRenderEdges;
- (BOOL)doRenderFaces;
- (BOOL)doRenderEdges;

- (void)render:(RenderContext *)renderContext;

- (void)validateFaces:(NSSet *)invalidFaces;
- (void)validate;

- (void)setFilter:(id <Filter>)theFilter;

@end

//
//  PolygonRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@class TextureManager;
@protocol Face;
@protocol RenderFilter;

@interface FaceRenderer : NSObject {
    @private
    NSMutableSet* faces;
    NSMutableDictionary* indexBuffers;
    NSMutableDictionary* countBuffers;
    id <RenderFilter> filter;
    VBOBuffer* vbo;
    TextureManager* textureManager;
    BOOL valid;
}

- (id)initWithTextureManager:(TextureManager *)theTextureManager;

- (void)addFace:(id <Face>)theFace;
- (void)removeFace:(id <Face>)theFace;

- (void)setFilter:(id <RenderFilter>)theFilter;
- (void)renderTextured:(BOOL)textured;

- (void)invalidate;

@end

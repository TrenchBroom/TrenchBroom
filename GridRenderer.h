//
//  GridRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@class TextureManager;
@protocol Face;
@protocol RenderFilter;

@interface GridRenderer : NSObject {
@private
    NSMutableSet* faces;
    VBOBuffer* vbo;
    int vertexCount;
    BOOL valid;
    int gridSize;
}

- (id)initWithGridSize:(int)theGridSize;

- (void)addFace:(id <Face>)theFace;
- (void)removeFace:(id <Face>)theFace;

- (void)setGridSize:(int)theGridSize;
- (void)render;

- (void)invalidate;

@end

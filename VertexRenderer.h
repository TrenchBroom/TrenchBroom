//
//  VertexRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@class Vertex;
@protocol RenderFilter;

@interface VertexRenderer : NSObject {
    NSMutableSet* vertices;
    int vertexCount;
    VBOBuffer* vbo;
    BOOL valid;
    id <RenderFilter> filter;
}

- (void)addVertex:(Vertex *)theVertex;
- (void)removeVertex:(Vertex *)theVertex;

- (void)setFilter:(id <RenderFilter>)theFilter;
- (void)render;

- (void)invalidate;

@end

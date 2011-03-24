//
//  ThickEdgeRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@class Edge;
@protocol RenderFilter;

@interface ThickEdgeRenderer : NSObject {
    NSMutableSet* edges;
    int vertexCount;
    VBOBuffer* vbo;
    BOOL valid;
    id <RenderFilter> filter;
}

- (void)addEdge:(Edge *)theEdge;
- (void)removeEdge:(Edge *)theEdge;

- (void)setFilter:(id <RenderFilter>)theFilter;
- (void)render;

- (void)invalidate;

@end

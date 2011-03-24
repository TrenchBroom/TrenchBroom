//
//  ThickEdgeRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ThickEdgeRenderer.h"
#import "Edge.h"
#import "Vertex.h"
#import "RenderFilter.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Vector3f.h"
#import "MathCache.h"

static int VerticesPerBlock = 2000;
static float Radius = 2.0f;
static int Segments = 12;

@interface ThickEdgeRenderer (private)

- (int)writeEdge:(Edge *)theEdge block:(VBOMemBlock *)theBlock offset:(int)offset;
- (void)validate;

@end

@implementation ThickEdgeRenderer (private)

- (int)writeEdge:(Edge *)theEdge block:(VBOMemBlock *)theBlock offset:(int)offset {
    Vector3f* startVertex = [[theEdge startVertex] vector];
    Vector3f* endVertex = [[theEdge endVertex] vector];
    
    MathCache* cache = [MathCache sharedCache];
    Vector3f* diff = [cache vector3f];
    [diff setFloat:endVertex];
    [diff sub:startVertex];
    
    Vector3f* v = [cache vector3f];
    
    [cache returnVector3f:diff];
    [cache returnVector3f:v];
}

- (void)validate {
    if (!valid) {
        [vbo deactivate];
        [vbo freeAllBlocks];
        
        [vbo activate];
        [vbo mapBuffer];
        
        VBOMemBlock* block = nil;
        int offset;
        vertexCount = 0;
        
        NSEnumerator* edgeEn = [edges objectEnumerator];
        Edge* edge;
        while ((edge = [edgeEn nextObject])) {
            if (filter == nil || [filter edgePasses:edge]) {
                if ((vertexCount % VerticesPerBlock) == 0) {
                    [block setState:BS_USED_VALID];
                    block = [vbo allocMemBlock:VerticesPerBlock * 3 * sizeof(float)];
                    offset = 0;
                }
                
                offset = [self writeEdge:edge block:block offset:offset];
                vertexCount += 2;
            }
        }
        
        if (block != nil)
            [block setState:BS_USED_VALID];
        // [vbo pack]; // probably unnecessary
        [vbo unmapBuffer];
        
        valid = YES;
    }
}

@end

@implementation ThickEdgeRenderer

- (id)init {
    if (self = [super init]) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        edges = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)addEdge:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    if (![edges containsObject:theEdge]) {
        [edges addObject:theEdge];
        [self invalidate];
    }
}

- (void)removeEdge:(Edge *)theEdge {
    NSAssert(theEdge != nil, @"edge must not be nil");
    if ([edges containsObject:theEdge]) {
        [edges removeObject:theEdge];
        [self invalidate];
    }
}

- (void)setFilter:(id <RenderFilter>)theFilter {
    if (filter == theFilter)
        return;
    
    [filter release];
    filter = [theFilter retain];
    [self invalidate];
}

- (void)render {
    [vbo activate];
    [self validate];
    
    glVertexPointer(3, GL_FLOAT, 0, NULL);
    glDrawArrays(GL_LINES, 0, vertexCount);
    
    [vbo deactivate];
}

- (void)invalidate {
    valid = NO;
}

- (void)dealloc {
    [filter release];
    [edges release];
    [vbo release];
    [super dealloc];
}

@end

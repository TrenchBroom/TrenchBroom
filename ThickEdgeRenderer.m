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
#import "Quaternion.h"
#import "MathCache.h"
#import "Math.h"

static float Radius = 1.0f;
static int Segments = 8;
static NSArray* Circle;
static int VerticesPerBlock;

@interface ThickEdgeRenderer (private)

- (int)writeEdge:(Edge *)theEdge block:(VBOMemBlock *)theBlock offset:(int)offset;
- (void)validate;

@end

@implementation ThickEdgeRenderer (private)

+ (void)initialize {
    Circle = [makeCircle(Radius, Segments) retain];
    VerticesPerBlock = 500 * 4 * Segments;
}

- (int)writeEdge:(Edge *)theEdge block:(VBOMemBlock *)theBlock offset:(int)offset {
    Vector3f* startVertex = [[theEdge startVertex] vector];
    Vector3f* endVertex = [[theEdge endVertex] vector];
    
    MathCache* cache = [MathCache sharedCache];
    Vector3f* diff = [cache vector3f];
    Vector3f* edgeAxis = [cache vector3f];
    Vector3f* rotAxis = [cache vector3f];
    Vector3f* v1 = [cache vector3f];
    Vector3f* v2 = [cache vector3f];
    Vector3f* v3 = [cache vector3f];
    Vector3f* v4 = [cache vector3f];
    Quaternion* rot = [cache quaternion];

    [diff setFloat:endVertex];
    [diff sub:startVertex];

    [edgeAxis setFloat:diff];
    [edgeAxis normalize];
    
    [rotAxis setFloat:diff];
    [rotAxis cross:[Vector3f zAxisPos]];
    
    BOOL needsRot = ![rotAxis isNull];
    if (needsRot) {
        [rotAxis normalize];
    
        float cos = [edgeAxis dot:[Vector3f zAxisPos]];
        [rot setAngle:acos(cos) axis:rotAxis];
    }
    
    [v1 setFloat:[Circle lastObject]];
    if (needsRot)
        [rot rotate:v1];
    [v1 add:startVertex];
    [v2 setFloat:v1];
    [v2 add:diff];
    
    for (int i = 0; i < [Circle count]; i++) {
        [v3 setFloat:[Circle objectAtIndex:i]];
        if (needsRot)
            [rot rotate:v3];
        [v3 add:startVertex];
        [v4 setFloat:v3];
        [v4 add:diff];
        
        offset = [theBlock writeVector3f:v1 offset:offset];
        offset = [theBlock writeVector3f:v2 offset:offset];
        offset = [theBlock writeVector3f:v4 offset:offset];
        offset = [theBlock writeVector3f:v3 offset:offset];

        [v1 setFloat:v3];
        [v2 setFloat:v4];
    }
    
    [cache returnVector3f:diff];
    [cache returnVector3f:edgeAxis];
    [cache returnVector3f:rotAxis];
    [cache returnQuaternion:rot];
    [cache returnVector3f:v1];
    [cache returnVector3f:v2];
    [cache returnVector3f:v3];
    [cache returnVector3f:v4];
    
    return offset;
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
                vertexCount += 4 * Segments;
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
    glDrawArrays(GL_QUADS, 0, vertexCount);
    
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

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
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:endVertex];
    [diff sub:startVertex];

    Vector3f* edgeAxis = [[Vector3f alloc] initWithFloatVector:diff];
    [edgeAxis normalize];
    
    Vector3f* rotAxis = [[Vector3f alloc] initWithFloatVector:diff];
    [rotAxis cross:[Vector3f zAxisPos]];
    
    Quaternion* rot = nil;
    if (![rotAxis isNull]) {
        [rotAxis normalize];
    
        float cos = [edgeAxis dot:[Vector3f zAxisPos]];
        rot = [[Quaternion alloc] initWithAngle:acos(cos) axis:rotAxis];
    }
    
    Vector3f* v1 = [[Vector3f alloc] init];
    Vector3f* v2 = [[Vector3f alloc] init];
    Vector3f* v3 = [[Vector3f alloc] init];
    Vector3f* v4 = [[Vector3f alloc] init];
    
    [v1 setFloat:[Circle lastObject]];
    if (rot != nil)
        [rot rotate:v1];
    [v1 add:startVertex];
    [v2 setFloat:v1];
    [v2 add:diff];
    
    for (int i = 0; i < [Circle count]; i++) {
        [v3 setFloat:[Circle objectAtIndex:i]];
        if (rot != nil)
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
    
    [diff release];
    [edgeAxis release];
    [rotAxis release];
    [rot release];
    [v1 release];
    [v2 release];
    [v3 release];
    [v4 release];
    
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

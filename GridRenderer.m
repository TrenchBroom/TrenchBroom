//
//  GridRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GridRenderer.h"
#import "Face.h"
#import "RenderFilter.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Vector3f.h"

@interface GridRenderer (private)

- (int)writeFace:(id <Face>)theFace;
- (void)validate;

@end

@implementation GridRenderer (private)

- (int)writeFace:(id <Face>)theFace {
    NSArray* gridVertices = [theFace gridWithSize:gridSize];
    VBOMemBlock* block = [vbo allocMemBlock:[gridVertices count] * 3 * sizeof(float)];

    int offset = 0;
    NSEnumerator* gridVertexEn = [gridVertices objectEnumerator];
    Vector3f* gridVertex;
    while ((gridVertex = [gridVertexEn nextObject]))
        offset = [block writeVector3f:gridVertex offset:offset];

    [block setState:BS_USED_VALID];
    return [gridVertices count];
}

- (void)validate {
    if (!valid) {
        [vbo deactivate];
        [vbo freeAllBlocks];
        
        [vbo activate];
        [vbo mapBuffer];
        
        vertexCount = 0;
        
        NSEnumerator* faceEn = [faces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject]))
            vertexCount += [self writeFace:face];

        [vbo pack]; // probably unnecessary
        [vbo unmapBuffer];
        
        valid = YES;
    }
}

@end

@implementation GridRenderer

- (id)init {
    if (self = [super init]) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        faces = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (id)initWithGridSize:(int)theGridSize {
    if (self = [self init]) {
        gridSize = theGridSize;
    }
    
    return self;
}

- (void)addFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    NSAssert(![faces containsObject:theFace], @"face is already handled by this renderer");
    
    [faces addObject:theFace];
    [self invalidate];
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    NSAssert([faces containsObject:theFace], @"face is not handled by this renderer");
    
    [faces removeObject:theFace];
    [self invalidate];
}

- (void)setGridSize:(int)theGridSize {
    if (gridSize == theGridSize)
        return;
    
    gridSize = theGridSize;
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
    [faces release];
    [vbo release];
    [super dealloc];
}

@end

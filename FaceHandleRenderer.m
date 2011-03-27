//
//  FaceHandleRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 27.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FaceHandleRenderer.h"
#import "Face.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Vector3f.h"

@interface FaceHandleRenderer (private)

- (int)writeFace:(id <Face>)theFace;
- (void)validate;

@end

@implementation FaceHandleRenderer (private)

- (int)writeFace:(id <Face>)theFace {
    NSArray* handleVertices = [theFace handleVertices];
    VBOMemBlock* block = [vbo allocMemBlock:6 * 4 * 3 * sizeof(float)];
    int offset = 0;
    
    // south face
    offset = [block writeVector3f:[handleVertices objectAtIndex:0] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:1] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:5] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:4] offset:offset];
    
    // north face
    offset = [block writeVector3f:[handleVertices objectAtIndex:7] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:3] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:2] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:6] offset:offset];

    // east face
    offset = [block writeVector3f:[handleVertices objectAtIndex:0] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:2] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:3] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:1] offset:offset];
    
    // west face
    offset = [block writeVector3f:[handleVertices objectAtIndex:7] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:6] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:4] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:5] offset:offset];
    
    // top face
    offset = [block writeVector3f:[handleVertices objectAtIndex:7] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:5] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:1] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:3] offset:offset];

    // bottom face
    offset = [block writeVector3f:[handleVertices objectAtIndex:0] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:4] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:6] offset:offset];
    offset = [block writeVector3f:[handleVertices objectAtIndex:2] offset:offset];
    
    [block setState:BS_USED_VALID];
    return 6 * 4;
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
        
        [vbo unmapBuffer];
        
        valid = YES;
    }
}

@end

@implementation FaceHandleRenderer

- (id)init {
    if (self = [super init]) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        faces = [[NSMutableSet alloc] init];
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
    [faces release];
    [vbo release];
    [super dealloc];
}

@end

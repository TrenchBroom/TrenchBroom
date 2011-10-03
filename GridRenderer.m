/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "GridRenderer.h"
#import "Face.h"
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
    
    if (![faces containsObject:theFace]) {
        [faces addObject:theFace];
        [self invalidate];
    }
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");

    if ([faces containsObject:theFace]) {
        [faces removeObject:theFace];
        [self invalidate];
    }
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

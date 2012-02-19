/*
Copyright (C) 2010-2012 Kristian Duske

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

#import "GLString.h"
#import "GLStringData.h"
#import "FloatData.h"
#import "IntData.h"

@interface GLString (private)

- (void)validateVbo;

@end

@implementation GLString (private)

- (void)validateVbo {
    NSAssert(!vboValid, @"vbo must be invalid");
    NSAssert(glStringData != nil, @"string data must not be nil");
    NSAssert(vbo != NULL, @"vbo must not be NULL");
    NSAssert(vbo->active, @"vbo must be active");
    
    BOOL wasMapped = vbo->mapped;
    if (!wasMapped)
        mapVbo(vbo);
    
    FloatData* triangleSet = [glStringData triangleSet];
    NSArray* triangleStrips = [glStringData triangleStrips];
    NSArray* triangleFans = [glStringData triangleFans];
    int vertexCount = [glStringData vertexCount];
    
    vboBlock = allocVboBlock(vbo, 3 * vertexCount * sizeof(float));
    hasTriangleSet = triangleSet != nil;
    hasTriangleStrips = triangleStrips != nil;
    hasTriangleFans = triangleFans != nil;
    
    int address = vboBlock->address;
    uint8_t* vboBuffer = vbo->buffer;
    
    if (hasTriangleSet) {
        triangleSetIndex = address / (3 * sizeof(float));
        triangleSetCount = [triangleSet count] / 3;
        const void* buffer = [triangleSet bytes];
        address = writeBuffer(buffer, vboBuffer, address, [triangleSet count] * sizeof(float));
    }
    
    if (hasTriangleStrips) {
        triangleStripIndices = [[IntData alloc] init];
        triangleStripCounts = [[IntData alloc] init];
        for (FloatData* strip in triangleStrips) {
            [triangleStripIndices appendInt:address / (3 * sizeof(float))];
            [triangleStripCounts appendInt:[strip count] / 3];
            const void* buffer = [strip bytes];
            address = writeBuffer(buffer, vboBuffer, address, [strip count] * sizeof(float));
        }
    }
    
    if (hasTriangleFans) {
        triangleFanIndices = [[IntData alloc] init];
        triangleFanCounts = [[IntData alloc] init];
        for (FloatData* fan in triangleFans) {
            [triangleFanIndices appendInt:address / (3 * sizeof(float))];
            [triangleFanCounts appendInt:[fan count] / 3];
            const void* buffer = [fan bytes];
            address = writeBuffer(buffer, vboBuffer, address, [fan count] * sizeof(float));
        }
    }
    
    if (!wasMapped)
        unmapVbo(vbo);
    
    [glStringData release];
    glStringData = nil;
    vbo = nil;
    vboValid = YES;
}

@end

@implementation GLString

- (id)initWithVbo:(Vbo *)theVbo data:(GLStringData *)theData size:(NSSize)theSize {
    if ((self = [self init])) {
        vbo = theVbo;
        size = theSize;
        glStringData = [theData retain];
        vboValid = NO;
        vboBlock = NULL;
    }
    
    return self;
}

- (NSSize)size {
    return size;
}

- (void)renderBackground {
    glBegin(GL_QUADS);
    glVertex3f(-1, -1, 0);
    glVertex3f(-1, size.height + 1, 0);
    glVertex3f(size.width + 1, size.height + 1, 0);
    glVertex3f(size.width + 1, -1, 0);
    glEnd();
}

- (void)render {
    if (!vboValid)
        [self validateVbo];
    if (hasTriangleSet)
        glDrawArrays(GL_TRIANGLES, triangleSetIndex, triangleSetCount);
    if (hasTriangleStrips)
        glMultiDrawArrays(GL_TRIANGLE_STRIP, [triangleStripIndices bytes], [triangleStripCounts bytes], [triangleStripIndices count]);
    if (hasTriangleFans)
        glMultiDrawArrays(GL_TRIANGLE_FAN, [triangleFanIndices bytes], [triangleFanCounts bytes], [triangleFanIndices count]);
}

- (void)dealloc {
    if (glStringData != nil)
        [glStringData release];
    if (vboBlock != NULL)
        freeVboBlock(vboBlock);
    [triangleStripIndices release];
    [triangleStripCounts release];
    [triangleFanIndices release];
    [triangleFanCounts release];
    [super dealloc];
}
@end

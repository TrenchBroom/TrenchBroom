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

@implementation GLString

- (id)initWithString:(NSString *)theString data:(GLStringData *)theData size:(NSSize)theSize cache:(NSMapTable *)theCache {
    NSAssert(theString != nil, @"string must not be nil");
    NSAssert(theData != nil, @"data must not be nil");
    NSAssert(theCache != nil, @"cache map must not be nil");
    
    if ((self = [self init])) {
        string = [theString retain];
        cache = [theCache retain];
        [cache setObject:self forKey:string];
        glStringData = [theData retain];
        size = theSize;
        vboBlock = NULL;
    }
    
    return self;
}

- (NSSize)size {
    return size;
}

- (void)prepare:(Vbo *)theVbo {
    NSAssert(theVbo != NULL, @"vbo must not be NULL");
    NSAssert(theVbo->mapped, @"vbo must be mapped");
    NSAssert(glStringData != nil, @"string data must not be nil");
    
    FloatData* triangleSet = [glStringData triangleSet];
    NSArray* triangleStrips = [glStringData triangleStrips];
    NSArray* triangleFans = [glStringData triangleFans];
    int vertexCount = [glStringData vertexCount];
    
    vboBlock = allocVboBlock(theVbo, 2 * vertexCount * sizeof(float));
    hasTriangleSet = triangleSet != nil;
    hasTriangleStrips = triangleStrips != nil;
    hasTriangleFans = triangleFans != nil;
    
    int address = vboBlock->address;
    uint8_t* vboBuffer = theVbo->buffer;
    
    if (hasTriangleSet) {
        triangleSetIndex = address / (2 * sizeof(float));
        triangleSetCount = [triangleSet count] / 2;
        const void* buffer = [triangleSet bytes];
        address = writeBuffer(buffer, vboBuffer, address, [triangleSet count] * sizeof(float));
    }
    
    if (hasTriangleStrips) {
        triangleStripIndices = [[IntData alloc] init];
        triangleStripCounts = [[IntData alloc] init];
        for (FloatData* strip in triangleStrips) {
            [triangleStripIndices appendInt:address / (2 * sizeof(float))];
            [triangleStripCounts appendInt:[strip count] / 2];
            const void* buffer = [strip bytes];
            address = writeBuffer(buffer, vboBuffer, address, [strip count] * sizeof(float));
        }
    }
    
    if (hasTriangleFans) {
        triangleFanIndices = [[IntData alloc] init];
        triangleFanCounts = [[IntData alloc] init];
        for (FloatData* fan in triangleFans) {
            [triangleFanIndices appendInt:address / (2 * sizeof(float))];
            [triangleFanCounts appendInt:[fan count] / 2];
            const void* buffer = [fan bytes];
            address = writeBuffer(buffer, vboBuffer, address, [fan count] * sizeof(float));
        }
    }
    
    [glStringData release];
    glStringData = nil;
}

- (void)renderBackground:(NSSize)theInsets {
    glBegin(GL_QUADS);
    glVertex3f(-theInsets.width, -theInsets.height, 0);
    glVertex3f(-theInsets.width, size.height + theInsets.height, 0);
    glVertex3f(size.width + theInsets.width, size.height + theInsets.height, 0);
    glVertex3f(size.width + theInsets.width, -theInsets.height, 0);
    glEnd();
}

- (void)render {
    NSAssert(vboBlock != NULL, @"vbo block must not be NULL");
    if (hasTriangleSet)
        glDrawArrays(GL_TRIANGLES, triangleSetIndex, triangleSetCount);
    if (hasTriangleStrips)
        glMultiDrawArrays(GL_TRIANGLE_STRIP, [triangleStripIndices bytes], [triangleStripCounts bytes], [triangleStripIndices count]);
    if (hasTriangleFans)
        glMultiDrawArrays(GL_TRIANGLE_FAN, [triangleFanIndices bytes], [triangleFanCounts bytes], [triangleFanIndices count]);
}

- (void)dealloc {
    [cache removeObjectForKey:string];
    [string release];
    [cache release];
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

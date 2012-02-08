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
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "FloatData.h"
#import "IntData.h"

@implementation GLString

- (id)initWithVbo:(VBOBuffer *)theVbo data:(GLStringData *)theData size:(NSSize)theSize {
    if ((self = [self init])) {
        size = theSize;

        FloatData* triangleSet = [theData triangleSet];
        NSArray* triangleStrips = [theData triangleStrips];
        NSArray* triangleFans = [theData triangleFans];
        int vertexCount = [theData vertexCount];
        
        memBlock = [theVbo allocMemBlock:2 * vertexCount * sizeof(float)];
        hasTriangleSet = triangleSet != nil;
        hasTriangleStrips = triangleStrips != nil;
        hasTriangleFans = triangleFans != nil;
        
        BOOL wasActive = [theVbo active];
        BOOL wasMapped = [theVbo mapped];
        
        if (!wasActive)
            [theVbo activate];
        if (!wasMapped)
            [theVbo mapBuffer];
        
        int address = [memBlock address];
        uint8_t* vboBuffer = [theVbo buffer];
        
        if (hasTriangleSet) {
            triangleSetIndex = [memBlock address] / (2 * sizeof(float));
            triangleSetCount = [triangleSet count] / 2;
            const void* buffer = [triangleSet bytes];
            address = writeBuffer(buffer, vboBuffer, address, [triangleSet count] * sizeof(float));
        }
        
        if (hasTriangleStrips) {
            triangleStripIndices = [[IntData alloc] init];
            triangleStripCounts = [[IntData alloc] init];
            NSEnumerator* stripEn = [triangleStrips objectEnumerator];
            FloatData* strip;
            while ((strip = [stripEn nextObject])) {
                [triangleStripIndices appendInt:address / (2 * sizeof(float))];
                [triangleStripCounts appendInt:[strip count] / 2];
                const void* buffer = [strip bytes];
                address = writeBuffer(buffer, vboBuffer, address, [strip count] * sizeof(float));
            }
        }
        
        if (hasTriangleFans) {
            triangleFanIndices = [[IntData alloc] init];
            triangleFanCounts = [[IntData alloc] init];
            NSEnumerator* fanEn = [triangleFans objectEnumerator];
            FloatData* fan;
            while ((fan = [fanEn nextObject])) {
                [triangleFanIndices appendInt:address / (2 * sizeof(float))];
                [triangleFanCounts appendInt:[fan count] / 2];
                const void* buffer = [fan bytes];
                address = writeBuffer(buffer, vboBuffer, address, [fan count] * sizeof(float));
            }
        }
        
        if (!wasMapped)
            [theVbo unmapBuffer];
        if (!wasActive)
            [theVbo deactivate];
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
    if (hasTriangleSet)
        glDrawArrays(GL_TRIANGLES, triangleSetIndex, triangleSetCount);
    if (hasTriangleStrips)
        glMultiDrawArrays(GL_TRIANGLE_STRIP, [triangleStripIndices bytes], [triangleStripCounts bytes], [triangleStripIndices count]);
    if (hasTriangleFans)
        glMultiDrawArrays(GL_TRIANGLE_FAN, [triangleFanIndices bytes], [triangleFanCounts bytes], [triangleFanIndices count]);
}

- (void)dealloc {
    [memBlock free];
    [triangleStripIndices release];
    [triangleStripCounts release];
    [triangleFanIndices release];
    [triangleFanCounts release];
    [super dealloc];
}
@end

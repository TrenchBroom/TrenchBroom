//
//  GLString.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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
        
        int offset = 0;
        if (hasTriangleSet) {
            triangleSetIndex = [memBlock address] / (2 * sizeof(float));
            triangleSetCount = [triangleSet count] / 2;
            const void* buffer = [triangleSet bytes];
            offset = [memBlock writeBuffer:buffer offset:offset count:[triangleSet count] * sizeof(float)];
        }
        
        if (hasTriangleStrips) {
            triangleStripIndices = [[IntData alloc] init];
            triangleStripCounts = [[IntData alloc] init];
            NSEnumerator* stripEn = [triangleStrips objectEnumerator];
            FloatData* strip;
            while ((strip = [stripEn nextObject])) {
                [triangleStripIndices appendInt:([memBlock address] + offset) / (2 * sizeof(float))];
                [triangleStripCounts appendInt:[strip count] / 2];
                const void* buffer = [strip bytes];
                offset = [memBlock writeBuffer:buffer offset:offset count:[strip count] * sizeof(float)];
            }
        }
        
        if (hasTriangleFans) {
            triangleFanIndices = [[IntData alloc] init];
            triangleFanCounts = [[IntData alloc] init];
            NSEnumerator* fanEn = [triangleFans objectEnumerator];
            FloatData* fan;
            while ((fan = [fanEn nextObject])) {
                [triangleFanIndices appendInt:([memBlock address] + offset) / (2 * sizeof(float))];
                [triangleFanCounts appendInt:[fan count] / 2];
                const void* buffer = [fan bytes];
                offset = [memBlock writeBuffer:buffer offset:offset count:[fan count] * sizeof(float)];
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

- (void)render {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);
    if (hasTriangleSet)
        glDrawArrays(GL_TRIANGLES, triangleSetIndex, triangleSetCount);
    if (hasTriangleStrips)
        glMultiDrawArrays(GL_TRIANGLE_STRIP, [triangleStripIndices bytes], [triangleStripCounts bytes], [triangleStripIndices count]);
    if (hasTriangleFans)
        glMultiDrawArrays(GL_TRIANGLE_FAN, [triangleFanIndices bytes], [triangleFanCounts bytes], [triangleFanIndices count]);
}

- (void)dealloc {
    [memBlock free];
    [memBlock release];
    [triangleStripIndices release];
    [triangleStripCounts release];
    [triangleFanIndices release];
    [triangleFanCounts release];
    [super dealloc];
}
@end

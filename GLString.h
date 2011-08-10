//
//  GLString.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLStringData;
@class VBOBuffer;
@class VBOMemBlock;
@class IntData;

@interface GLString : NSObject {
    @private
    VBOMemBlock* memBlock;
    BOOL hasTriangleSet;
    int triangleSetIndex;
    int triangleSetCount;
    BOOL hasTriangleStrips;
    IntData* triangleStripIndices;
    IntData* triangleStripCounts;
    BOOL hasTriangleFans;
    IntData* triangleFanIndices;
    IntData* triangleFanCounts;
    NSSize size;
}

- (id)initWithVbo:(VBOBuffer *)theVbo data:(GLStringData *)theData size:(NSSize)theSize;

- (NSSize)size;

- (void)renderBackground;
- (void)render;
@end

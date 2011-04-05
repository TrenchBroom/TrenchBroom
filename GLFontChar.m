//
//  GLFontChar.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLFontChar.h"
#import "VBOMemBlock.h"

@implementation GLFontChar
- (id)initWithDimensions:(NSSize)theDimensions {
    if (self = [self init]) {
        dimensions = theDimensions;
    }
    
    return self;
}

- (void)calculateTexCoordsForTexSize:(NSSize)theTexSize charPos:(NSPoint)theCharPos {
    s1 = theCharPos.x / theTexSize.width;
    t1 = theCharPos.y / theTexSize.height;
    s2 = (theCharPos.x + dimensions.width) / theTexSize.width;
    t2 = (theCharPos.y + dimensions.height) / theTexSize.height;
}

- (int)renderAt:(NSPoint)thePosition intoVBO:(VBOMemBlock *)theMemBlock offset:(int)theOffset {
    int offset = [theMemBlock writeFloat:s1 offset:theOffset];
    offset = [theMemBlock writeFloat:t2 offset:offset];
    offset = [theMemBlock writeFloat:thePosition.x offset:offset];
    offset = [theMemBlock writeFloat:thePosition.y offset:offset];
    offset = [theMemBlock writeFloat:0 offset:offset];

    offset = [theMemBlock writeFloat:s1 offset:offset];
    offset = [theMemBlock writeFloat:t1 offset:offset];
    offset = [theMemBlock writeFloat:thePosition.x offset:offset];
    offset = [theMemBlock writeFloat:thePosition.y + dimensions.height offset:offset];
    offset = [theMemBlock writeFloat:0 offset:offset];
    
    offset = [theMemBlock writeFloat:s2 offset:offset];
    offset = [theMemBlock writeFloat:t1 offset:offset];
    offset = [theMemBlock writeFloat:thePosition.x + dimensions.width offset:offset];
    offset = [theMemBlock writeFloat:thePosition.y + dimensions.height offset:offset];
    offset = [theMemBlock writeFloat:0 offset:offset];
    
    offset = [theMemBlock writeFloat:s2 offset:offset];
    offset = [theMemBlock writeFloat:t2 offset:offset];
    offset = [theMemBlock writeFloat:thePosition.x + dimensions.width offset:offset];
    offset = [theMemBlock writeFloat:thePosition.y offset:offset];
    offset = [theMemBlock writeFloat:0 offset:offset];
    
    return offset;
}

@end

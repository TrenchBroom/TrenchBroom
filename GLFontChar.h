//
//  GLFontChar.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;
@class VBOMemBlock;

@interface GLFontChar : NSObject {
    float s1;
    float t1;
    float s2;
    float t2;
    NSSize dimensions;
}

- (id)initWithDimensions:(NSSize)theDimensions;
- (void)calculateTexCoordsForTexSize:(NSSize)theTexSize charPos:(NSPoint)theCharPos;

- (int)renderAt:(NSPoint)thePosition intoVBO:(VBOMemBlock *)theMemBlock offset:(int)theOffset;
@end

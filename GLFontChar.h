//
//  GLFontChar.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface GLFontChar : NSObject {
    float s1;
    float t1;
    float s2;
    float t2;
    NSSize dimensions;
}

- (id)initWithDimensions:(NSSize)theDimensions;
- (void)calculateTexCoordsForTexSize:(NSSize)theTexSize charPos:(NSPoint)theCharPos;

- (void)render;
@end

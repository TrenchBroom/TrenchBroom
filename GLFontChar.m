//
//  GLFontChar.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLFontChar.h"


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

- (void)render {
    glBegin(GL_QUADS);
    glTexCoord2f(s1, t1);
    glVertex2f(0, 0);
    glTexCoord2f(s2, t1);
    glVertex2f(dimensions.width, 0);
    glTexCoord2f(s2, t2);
    glVertex2f(dimensions.width, dimensions.height);
    glTexCoord2f(s1, t2);
    glVertex2f(0, dimensions.height);
    glEnd();
}

@end

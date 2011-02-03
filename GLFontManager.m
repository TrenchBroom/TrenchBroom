//
//  GLFontManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLFontManager.h"
#import "GLFont.h"
#import "VBOBuffer.h"

@implementation GLFontManager

- (id)init {
    if (self = [super init]) {
        fonts = [[NSMutableDictionary alloc] init];
        stringVBO = [[VBOBuffer alloc] initWithTotalCapacity:8192];
    }
    
    return self;
}

- (GLFont *)glFontFor:(NSFont *)theFont {
    GLFont* glFont = [fonts objectForKey:theFont];
    if (glFont == nil) {
        glFont = [[GLFont alloc] initWithFont:theFont stringVBO:stringVBO];
        [fonts setObject:glFont forKey:theFont];
        [glFont release];
    }
    
    return glFont;
}

- (void)dispose {
    NSEnumerator* fontEn = [fonts objectEnumerator];
    GLFont* glFont;
    while ((glFont = [fontEn nextObject]))
        [glFont dispose];

    [fonts removeAllObjects];
    [stringVBO dispose];
}

- (void)dealloc {
    [fonts release];
    [stringVBO release];
    [super dealloc];
}
@end

//
//  GLFontManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@class GLFont;

@interface GLFontManager : NSObject {
    @private
    NSMutableDictionary* fonts;
    VBOBuffer* stringVBO;
}

- (GLFont *)glFontFor:(NSFont *)theFont;

- (void)dispose;
@end

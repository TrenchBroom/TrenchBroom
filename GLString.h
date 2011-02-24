//
//  GLString.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLFont;
@class VBOMemBlock;

@interface GLString : NSObject {
    @private
    GLFont* glFont;
    VBOMemBlock* memBlock;
    int length;
}

- (id)initWithMemBlock:(VBOMemBlock *)theMemBlock glFont:(GLFont *)theFont;

- (void)render;
@end

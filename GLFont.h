//
//  GLFont.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLString;
@class VBOBuffer;

@interface GLFont : NSObject {
    @private
    NSMutableArray* chars;
    GLuint texId;
    NSSize texSize;
    NSLayoutManager* layoutManager;
    NSTextStorage* textStorage;
    NSTextContainer* textContainer;
    VBOBuffer* stringVBO;
}

- (id)initWithFont:(NSFont *)theFont stringVBO:(VBOBuffer *)theStringVBO;

- (GLString *)glStringFor:(NSString *)theString;

- (void)activate;
- (void)deactivate;

- (void)dispose;
@end

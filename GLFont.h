//
//  GLFont.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface GLFont : NSObject {
    @private
    NSMutableArray* chars;
    GLuint texId;
    NSSize texSize;
    NSLayoutManager* layoutManager;
    NSTextStorage* textStorage;
    NSTextContainer* textContainer;
}

- (id)initWithFont:(NSFont *)theFont;

- (void)renderString:(NSString *)theString;

- (void)dispose;
@end

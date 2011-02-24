//
//  GLFont.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLString;

@interface GLFont : NSObject {
    @private
    NSMutableArray* chars;
    GLuint texId;
    NSSize texSize;
    NSLayoutManager* layoutManager;
    NSTextStorage* textStorage;
    NSTextContainer* textContainer;
    NSMutableArray* vbos;
}

- (id)initWithFont:(NSFont *)theFont;

- (GLString *)glStringFor:(NSString *)theString;
- (NSSize)sizeOfString:(NSString *)theString;

- (void)activate;
- (void)deactivate;
@end

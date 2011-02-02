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
    NSArray* chars;
    GLuint texId;
}

- (id)initWithFont:(NSFont *)theFont;

@end

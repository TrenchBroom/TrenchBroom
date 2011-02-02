//
//  GLString.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface GLString : NSObject {
    @private
    NSString* string;
    GLuint texId;
    NSSize texSize;
}

- (id)initWithString:(NSString *)theString;

- (void)render;

- (void)dispose;

@end

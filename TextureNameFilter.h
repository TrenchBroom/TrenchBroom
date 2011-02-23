//
//  TextureNameFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TextureFilter.h"

@interface TextureNameFilter : NSObject <TextureFilter> {
    NSString* pattern;
    id<TextureFilter> filter;
}

- (id)initWithPattern:(NSString *)thePattern;
- (id)initWithPattern:(NSString *)thePattern filter:(id<TextureFilter>)theFilter;

@end

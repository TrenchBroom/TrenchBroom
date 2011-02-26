//
//  TextureUsageFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TextureFilter.h"

@interface TextureUsageFilter : NSObject <TextureFilter> {
    id<TextureFilter> filter;
}

- (id)initWithFilter:(id<TextureFilter>)theFilter;

@end

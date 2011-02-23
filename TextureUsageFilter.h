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
    NSSet* names;
    id<TextureFilter> filter;
}

- (id)initWithTextureNames:(NSSet *)theNames;
- (id)initWithTextureNames:(NSSet *)theNames filter:(id<TextureFilter>)theFilter;

@end

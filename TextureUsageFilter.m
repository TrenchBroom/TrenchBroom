//
//  TextureUsageFilter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureUsageFilter.h"


@implementation TextureUsageFilter

- (id)initWithTextureNames:(NSSet *)theNames {
    if (theNames == nil)
        [NSException raise:NSInvalidArgumentException format:@"name set must not be nil"];
    
    if (self = [self init]) {
        names = [theNames retain];
    }
    
    return self;
}

- (id)initWithTextureNames:(NSSet *)theNames filter:(id<TextureFilter>)theFilter {
    if (self = [self initWithTextureNames:theNames]) {
        filter = [theFilter retain];
    }
    
    return self;
}

- (BOOL)passes:(Texture *)texture {
    if (filter != nil && ![filter passes:texture])
        return NO;
    
    return [names containsObject:[texture name]];
}

- (void)dealloc {
    [names release];
    [filter release];
    [super dealloc];
}


@end

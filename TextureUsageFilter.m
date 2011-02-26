//
//  TextureUsageFilter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureUsageFilter.h"


@implementation TextureUsageFilter

- (id)initWithFilter:(id<TextureFilter>)theFilter {
    if (self = [self init]) {
        filter = [theFilter retain];
    }
    
    return self;
}

- (BOOL)passes:(Texture *)texture {
    if (filter != nil && ![filter passes:texture])
        return NO;
    
    return [texture usageCount] > 0;
}

- (void)dealloc {
    [filter release];
    [super dealloc];
}


@end

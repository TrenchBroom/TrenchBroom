//
//  TextureNameFilter.m
//  TrenchBroom
//
//  Created by Kristian Duske on 23.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureNameFilter.h"
#import "Texture.h"

@implementation TextureNameFilter

- (id)initWithPattern:(NSString *)thePattern {
    if (self = [self init]) {
        pattern = [thePattern retain];
    }
    
    return self;
}

- (id)initWithPattern:(NSString *)thePattern filter:(id<TextureFilter>)theFilter {
    if (self = [self initWithPattern:thePattern]) {
        filter = [theFilter retain];
    }
    
    return self;
}

- (BOOL)passes:(Texture *)texture {
    if (filter != nil && ![filter passes:texture])
        return NO;
    
    return [[texture name] rangeOfString:pattern].location != NSNotFound;
}

- (void)dealloc {
    [pattern release];
    [filter release];
    [super dealloc];
}

@end

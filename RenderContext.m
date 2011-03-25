//
//  RenderContext.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RenderContext.h"
#import "TextureManager.h"
#import "Options.h"

@implementation RenderContext

- (id)initWithOptions:(Options *)theOptions {
    if (self = [self init]) {
        options = [theOptions retain];
    }
    
    return self;
}

- (Options *)options {
    return options;
}

- (void)dealloc {
    [options release];
    [super dealloc];
}

@end

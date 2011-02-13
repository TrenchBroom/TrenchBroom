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

- (id)initWithTextureManager:(TextureManager *)theTextureManager options:(Options *)theOptions {
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    if (theOptions == nil)
        [NSException raise:NSInvalidArgumentException format:@"options must not be nil"];
    
    if (self = [self init]) {
        textureManager = [theTextureManager retain];
        options = [theOptions retain];
    }
    
    return self;
}

- (TextureManager *)textureManager {
    return textureManager;
}

- (Options *)options {
    return options;
}

- (void)dealloc {
    [textureManager release];
    [options release];
    [super dealloc];
}

@end

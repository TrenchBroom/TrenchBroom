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

- (id)initWithTextureManager:(TextureManager *)theTextureManager vbo:(VBOBuffer *)theVbo options:(Options *)theOptions {
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    if (theVbo == nil)
        [NSException raise:NSInvalidArgumentException format:@"vbo must not be nil"];
    if (theOptions == nil)
        [NSException raise:NSInvalidArgumentException format:@"options must not be nil"];
    
    if (self = [self init]) {
        textureManager = [theTextureManager retain];
        vbo = [theVbo retain];
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

- (VBOBuffer *)vbo {
    return vbo;
}

- (void)dealloc {
    [textureManager release];
    [vbo release];
    [options release];
    [super dealloc];
}

@end

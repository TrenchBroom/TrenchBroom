//
//  RenderContext.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RenderContext.h"
#import "TextureManager.h"

@implementation RenderContext

- (id)initWithTextureManager:(TextureManager *)theTextureManager mode:(ERenderMode)theMode {
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    
    if (self = [self init]) {
        textureManager = [theTextureManager retain];
        mode = theMode;
    }
    
    return self;
}

- (TextureManager *)textureManager {
    return textureManager;
}

- (ERenderMode)mode {
    return mode;
}

- (void)dealloc {
    [textureManager release];
    [super dealloc];
}

@end

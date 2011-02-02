//
//  TextureWindowController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureWindowController.h"
#import "TextureView.h"
#import "TextureManager.h"

@implementation TextureWindowController

- (id)initWithWindowNibName:(NSString *)theWindowNibName sharedContext:(NSOpenGLContext *)theSharedContext textureManager:(TextureManager *)theTextureManager {
    if (theSharedContext == nil)
        [NSException raise:NSInvalidArgumentException format:@"shared context must not be nil"];
    if (theTextureManager == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture manager must not be nil"];
    
    if (self = [super initWithWindowNibName:theWindowNibName]) {
        sharedContext = [theSharedContext retain];
        textureManager = [theTextureManager retain];
    }
    
    return self;
}

- (void)windowDidLoad {
    [textureView switchToContext:sharedContext textureManager:textureManager];
}

- (void)dealloc {
    [sharedContext release];
    [textureManager release];
    [super dealloc];
}

@end

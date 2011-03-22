//
//  GLResources.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLResources.h"
#import "GLFontManager.h"
#import "TextureManager.h"
#import "VBOBuffer.h"

@implementation GLResources

- (id)init {
    if (self = [super init]) {
        NSOpenGLPixelFormatAttribute attrs[] = {0};
        NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
        [pixelFormat release];

        [openGLContext makeCurrentContext];
        
        fontManager = [[GLFontManager alloc] init];
        faceVbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        
        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        NSData* palette = [[NSData alloc] initWithContentsOfFile:palettePath];
        
        textureManager = [[TextureManager alloc] initWithPalette:palette];
        [palette release];
    }
    
    return self;
}

- (NSOpenGLContext *)openGLContext {
    return openGLContext;
}

- (GLFontManager *)fontManager {
    return fontManager;
}

- (TextureManager *)textureManager {
    return textureManager;
}

- (VBOBuffer *)faceVbo {
    return faceVbo;
}

- (void)dealloc {
    [fontManager release];
    [textureManager release];
    [faceVbo release];
    [openGLContext release];
    [super dealloc];
}

@end

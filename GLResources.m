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
        vbos = [[NSMutableDictionary alloc] init];
        
        textureManager = [[TextureManager alloc] init];
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

- (VBOBuffer *)vboForKey:(id <NSCopying>)theKey {
    VBOBuffer* vbo = [vbos objectForKey:theKey];
    if (vbo == nil) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        [vbos setObject:vbo forKey:theKey];
        [vbo release];
    }
    
    return vbo;
}

- (void)dealloc {
    [fontManager release];
    [textureManager release];
    [vbos release];
    [openGLContext release];
    [super dealloc];
}

@end

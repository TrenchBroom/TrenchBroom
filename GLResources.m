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
#import "EntityRendererManager.h"

@implementation GLResources

- (id)initWithPalette:(NSData *)thePalette {
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [super init])) {
        NSOpenGLPixelFormatAttribute attrs[] = {0};
        NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
        [pixelFormat release];
        
        palette = [thePalette retain];
        fontManager = [[GLFontManager alloc] init];
        textureManager = [[TextureManager alloc] init];
        entityRendererManager = [[EntityRendererManager alloc] initWithPalette:palette];
    }
    
    return self;
}

- (void)dealloc {
    [entityRendererManager release];
    [fontManager release];
    [textureManager release];
    [palette release];
    [geometryVbo release];
    [openGLContext release];
    [super dealloc];
}

- (NSOpenGLContext *)openGLContext {
    return openGLContext;
}

- (NSData *)palette {
    return palette;
}

- (GLFontManager *)fontManager {
    return fontManager;
}

- (TextureManager *)textureManager {
    return textureManager;
}

- (EntityRendererManager *)entityRendererManager {
    return entityRendererManager;
}

- (VBOBuffer *)geometryVbo {
    if (geometryVbo == nil)
        geometryVbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
    
    return geometryVbo;
}

@end

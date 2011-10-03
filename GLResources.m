/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
        vbos = [[NSMutableDictionary alloc] init];
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
    [vbos release];
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

- (VBOBuffer *)vboForKey:(id <NSCopying>)theKey {
    VBOBuffer* vbo = [vbos objectForKey:theKey];
    if (vbo == nil) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        [vbos setObject:vbo forKey:theKey];
        [vbo release];
    }
    
    return vbo;
}

- (void)reset {
    [textureManager clear];
//    [vbos removeAllObjects];
    [fontManager clear];
    [entityRendererManager clear];
}

@end

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

#import <Cocoa/Cocoa.h>

@class GLFontManager;
@class TextureManager;
@class VBOBuffer;
@class EntityRendererManager;

@interface GLResources : NSObject {
    @private
    NSOpenGLContext* openGLContext;
    GLFontManager* fontManager;
    TextureManager* textureManager;
    EntityRendererManager* entityRendererManager;
    NSMutableDictionary* vbos;
    NSData* palette;
}

- (id)initWithPalette:(NSData *)thePalette;

- (NSOpenGLContext *)openGLContext;
- (NSData *)palette;
- (GLFontManager *)fontManager;
- (TextureManager *)textureManager;
- (EntityRendererManager *)entityRendererManager;
- (VBOBuffer *)vboForKey:(id <NSCopying>)theKey;
- (void)reset;

@end

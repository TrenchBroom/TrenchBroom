//
//  GLResources.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLFontManager;
@class TextureManager;
@class VBOBuffer;

@interface GLResources : NSObject {
    @private
    NSOpenGLContext* openGLContext;
    GLFontManager* fontManager;
    TextureManager* textureManager;
    NSMutableDictionary* vbos;
}

- (NSOpenGLContext *)openGLContext;
- (GLFontManager *)fontManager;
- (TextureManager *)textureManager;
- (VBOBuffer *)vboForKey:(id <NSCopying>)theKey;

@end

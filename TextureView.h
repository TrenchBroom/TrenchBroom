//
//  TextureView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class TextureManager;
@class TextureViewLayout;
@class GLFontManager;

@interface TextureView : NSOpenGLView {
    @private
    TextureManager* textureManager;
    TextureViewLayout* layout;
    GLFontManager* fontManager;
}

- (void)switchToContext:(NSOpenGLContext *)theSharedContext textureManager:(TextureManager *)theTextureManager fontManager:(GLFontManager *)theFontManager;

@end

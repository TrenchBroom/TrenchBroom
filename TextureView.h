//
//  TextureView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TextureFilter.h"

@class TextureManager;
@class TextureViewLayout;
@class GLFontManager;
@class Map;

@interface TextureView : NSOpenGLView {
    @private
    TextureManager* textureManager;
    TextureViewLayout* layout;
    GLFontManager* fontManager;
    Map* map;
    NSSet* selectedTextureNames;
    NSMutableDictionary* glStrings;
}

- (void)switchToContext:(NSOpenGLContext *)theSharedContext textureManager:(TextureManager *)theTextureManager fontManager:(GLFontManager *)theFontManager map:(Map *)theMap;

- (void)setTextureFilter:(id <TextureFilter>)theFilter;
- (void)setSelectedTextureNames:(NSSet *)theNames;
@end

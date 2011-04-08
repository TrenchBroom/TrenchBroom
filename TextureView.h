//
//  TextureView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TextureFilter.h"
#import "TextureManager.h"

@class TextureViewLayout;
@class GLResources;

@interface TextureView : NSOpenGLView {
    @private
    TextureViewLayout* layout;
    NSSet* selectedTextureNames;
    IBOutlet id target;
    GLResources* glResources;
}

- (void)setTextureFilter:(id <TextureFilter>)theFilter;
- (void)setSelectedTextureNames:(NSSet *)theNames;
- (void)setGLResources:(GLResources *)theGLResources;

@end

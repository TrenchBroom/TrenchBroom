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
    ETextureSortCriterion sortCriterion;
}

- (void)setTextureFilter:(id <TextureFilter>)theFilter;
- (void)setSortCriterion:(ETextureSortCriterion)criterion;
- (void)setSelectedTextureNames:(NSSet *)theNames;
- (void)setGLResources:(GLResources *)theGLResources;

@end

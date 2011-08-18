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
@class SelectionManager;
@protocol TextureViewTarget;

@interface TextureView : NSOpenGLView {
    @private
    SelectionManager* selectionManager;
    TextureViewLayout* layout;
    IBOutlet id <TextureViewTarget> target;
    GLResources* glResources;
    ETextureSortCriterion sortCriterion;
}

- (void)setTextureFilter:(id <TextureFilter>)theFilter;
- (void)setSortCriterion:(ETextureSortCriterion)criterion;
- (void)setGLResources:(GLResources *)theGLResources;
- (void)setSelectionManager:(SelectionManager *)theSelectionManager;

@end

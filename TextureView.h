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
    NSMutableDictionary* glStrings;
    IBOutlet id target;
    GLResources* glResources;
    ESortCriterion sortCriterion;
}

- (void)setTextureFilter:(id <TextureFilter>)theFilter;
- (void)setSortCriterion:(ESortCriterion)theSortCriterion;
- (void)setSelectedTextureNames:(NSSet *)theNames;
- (void)setGLResources:(GLResources *)theGLResources;

@end

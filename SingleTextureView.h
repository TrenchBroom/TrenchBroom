//
//  SingleTextureView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLResources;

@interface SingleTextureView : NSOpenGLView {
    @private
    GLResources* glResources;
    NSString* textureName;
}

- (void)setGLResources:(GLResources *)theGlResources;
- (void)setTextureName:(NSString *)theTextureName;

@end

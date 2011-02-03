//
//  SingleTextureView.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Texture;

@interface SingleTextureView : NSOpenGLView {
    @private
    Texture* texture;
}

- (void)setTexture:(Texture *)theTexture;

@end

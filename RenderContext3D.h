//
//  RenderContext3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RenderContext.h"

@class TextureManager;

@interface RenderContext3D : NSObject <RenderContext> {
    @private 
    TextureManager* textureManager;
}

@end

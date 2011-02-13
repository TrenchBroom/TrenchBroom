//
//  RenderContext.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class TextureManager;
@class Options;

@interface RenderContext : NSObject {
    @private
    TextureManager* textureManager;
    Options* options;
}

- (id)initWithTextureManager:(TextureManager *)theTextureManager options:(Options *)theOptions;

- (TextureManager *)textureManager;
- (Options *)options;

@end

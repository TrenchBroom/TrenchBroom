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
@class VBOBuffer;

@interface RenderContext : NSObject {
    @private
    TextureManager* textureManager;
    Options* options;
    VBOBuffer* vbo;
}

- (id)initWithTextureManager:(TextureManager *)theTextureManager vbo:(VBOBuffer *)theVbo options:(Options *)theOptions;

- (TextureManager *)textureManager;
- (VBOBuffer *)vbo;
- (Options *)options;

@end

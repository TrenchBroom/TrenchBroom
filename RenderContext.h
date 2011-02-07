//
//  RenderContext.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    RM_TEXTURED,
    RM_FLAT,
    RM_WIREFRAME
} ERenderMode;

@class TextureManager;

@interface RenderContext : NSObject {
    @private
    TextureManager* textureManager;
    ERenderMode mode;
}

- (id)initWithTextureManager:(TextureManager *)theTextureManager mode:(ERenderMode)theMode;

- (TextureManager *)textureManager;
- (ERenderMode)mode;

@end

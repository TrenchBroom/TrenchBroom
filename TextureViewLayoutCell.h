//
//  TextureViewCell.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Texture;

@interface TextureViewLayoutCell : NSObject {
    @private
    Texture* texture;
    float x;
}

- (id)initAtX:(float)xPos texture:(Texture *)theTexture;

- (float)x;
- (float)textureWidth;
- (float)textureHeight;
- (Texture *)texture;

@end

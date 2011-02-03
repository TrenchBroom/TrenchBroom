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
    NSSize cellSize;
    NSRect textRect;
}

- (id)initAtX:(float)xPos texture:(Texture *)theTexture nameSize:(NSSize)theNameSize;

- (float)x;
- (float)textureWidth;
- (float)textureHeight;

- (NSSize)cellSize;

- (Texture *)texture;

@end

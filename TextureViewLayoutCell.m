//
//  TextureViewCell.m
//  TrenchBroom
//
//  Created by Kristian Duske on 22.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureViewLayoutCell.h"
#import "Texture.h"

@implementation TextureViewLayoutCell

- (id)initAtX:(float)xPos texture:(Texture *)theTexture {
    if (theTexture == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture must not be nil"];
    
    if (self = [self init]) {
        x = xPos;
        texture = [theTexture retain];
    }
    
    return self;
}

- (float)x {
    return x;
}

- (float)textureWidth {
    return [texture width];
}

- (float)textureHeight {
    return [texture height];
}

- (Texture *)texture {
    return texture;
}

- (void)dealloc {
    [texture release];
    [super dealloc];
}

@end

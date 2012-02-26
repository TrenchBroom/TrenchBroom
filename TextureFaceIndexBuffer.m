//
//  TextureFaceIndexBuffer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.12.
//  Copyright (c) 2012 TU Berlin. All rights reserved.
//

#import "TextureFaceIndexBuffer.h"
#import "Texture.h"

@implementation TextureFaceIndexBuffer

- (id)init {
    if ((self = [super init])) {
        texture = nil;
        initIndexBuffer(&indexBuffer, 0xFF);
    }
    
    return self;
}

- (void)dealloc {
    freeIndexBuffer(&indexBuffer);
    [super dealloc];
}

- (void)setTexture:(Texture *)theTexture {
    texture = theTexture;
}

- (Texture *)texture {
    return texture;
}

- (TIndexBuffer *)buffer {
    return &indexBuffer;
}

@end

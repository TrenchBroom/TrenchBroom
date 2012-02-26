//
//  TextureFaceIndexBuffer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.02.12.
//  Copyright (c) 2012 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Renderer.h"

@class Texture;

@interface TextureFaceIndexBuffer : NSObject {
    Texture* texture;
    TIndexBuffer indexBuffer;
}

- (void)setTexture:(Texture *)theTexture;
- (Texture *)texture;

- (TIndexBuffer *)buffer;

@end

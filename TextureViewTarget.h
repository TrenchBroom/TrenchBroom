//
//  TextureViewTarget.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class Texture;

@protocol TextureViewTarget <NSObject>

- (void)textureSelected:(Texture *)texture;

@end

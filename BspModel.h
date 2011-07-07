//
//  Bsp.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"

@class Texture;

@interface BspModel : NSObject {
@private
    NSArray* faces;
    NSArray* textures;
}

- (id)initWithFaces:(NSArray *)theFaces textures:(NSArray *)theTextures;

@end

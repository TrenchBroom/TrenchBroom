//
//  BspRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "EntityRenderer.h"

@class Bsp;
@class VBOBuffer;
@class VBOMemBlock;

@interface BspRenderer : NSObject <EntityRenderer> {
@private
    Bsp* bsp;
    VBOBuffer* vbo;
    VBOMemBlock* block;
    NSMutableSet* textures;
    NSMutableDictionary* indices;
    NSMutableDictionary* counts;
}

- (id)initWithBsp:(Bsp *)theBsp vbo:(VBOBuffer *)theVbo;

@end

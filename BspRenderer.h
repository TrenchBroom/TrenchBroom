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
    NSData* palette;
    VBOBuffer* vbo;
    VBOMemBlock* block;
    NSMutableDictionary* textures;
    NSMutableDictionary* indices;
    NSMutableDictionary* counts;
}

- (id)initWithBsp:(Bsp *)theBsp vbo:(VBOBuffer *)theVbo palette:(NSData *)thePalette;

@end

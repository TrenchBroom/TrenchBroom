//
//  AliasRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EntityRenderer.h"

@class Alias;
@class VBOBuffer;
@class VBOMemBlock;
@class Texture;

@interface AliasRenderer : NSObject <EntityRenderer> {
    Alias* alias;
    int skinIndex;
    VBOBuffer* vbo;
    VBOMemBlock* block;
    Texture* texture;
    NSData* palette;
    int triangleCount;
}

- (id)initWithAlias:(Alias *)theAlias skinIndex:(int)theSkinIndex vbo:(VBOBuffer *)theVbo palette:(NSData *)thePalette;
@end

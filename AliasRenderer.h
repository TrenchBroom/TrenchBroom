//
//  AliasRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Alias;
@class VBOBuffer;
@class VBOMemBlock;
@class Texture;
@protocol Entity;

@interface AliasRenderer : NSObject {
    Alias* alias;
    VBOBuffer* vbo;
    VBOMemBlock* block;
    Texture* texture;
    NSData* palette;
    int triangleCount;
}

- (id)initWithAlias:(Alias *)theAlias vbo:(VBOBuffer *)theVbo palette:(NSData *)thePalette;

- (void)renderWithEntity:(id <Entity>)theEntity;
@end

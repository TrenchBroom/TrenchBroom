//
//  RenderBrush.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RenderContext.h"

@class Brush;
@class VBOBuffer;
@class VBOMemBlock;
@class TextureManager;

@interface RenderBrush : NSObject {
    @private
    Brush* brush;
    VBOBuffer* vboBuffer;
    VBOMemBlock* vboMemBlock;
    NSMutableDictionary* arrayInfoForTexture;
}

- (id)initWithBrush:(Brush *)theBrush vboBuffer:(VBOBuffer *)theVboBuffer;

- (Brush *)brush;

- (void)renderWithContext:(id <RenderContext>)renderContext;

- (NSDictionary *)prepareWithTextureManager:(TextureManager *)theTextureManager;
@end

//
//  Figure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 11.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@class TextureManager;
@class IntData;

@protocol Figure <NSObject>

- (id)object;
- (void)prepareWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager;
- (void)getIndex:(IntData *)theIndexBuffer count:(IntData *)theCountBuffer;

- (void)invalidate;

@end

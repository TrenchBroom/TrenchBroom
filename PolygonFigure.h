//
//  PolygonFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "Figure.h"

@class VBOBuffer;
@class TextureManager;
@class IntData;

@protocol PolygonFigure <Figure>

- (NSString *)texture;
- (void)prepareWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager;
- (void)getIndex:(IntData *)theIndexBuffer count:(IntData *)theCountBuffer;

@end

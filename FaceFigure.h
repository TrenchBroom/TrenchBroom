//
//  FaceFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "PolygonFigure.h"

@protocol Face;
@class TextureManager;
@class VBOBuffer;
@class VBOMemBlock;

@interface FaceFigure : NSObject <PolygonFigure> {
    @private
    id <Face> face;
    TextureManager* textureManager;
    VBOMemBlock* block;
    int vboIndex;
    int vboCount;
}

- (id)initWithFace:(id <Face>)theFace textureManager:(TextureManager *)theTextureManager;

@end

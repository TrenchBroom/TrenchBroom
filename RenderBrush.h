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
@class Face;
@class IntData;
@class VBOBuffer;
@class VBOMemBlock;
@class TextureManager;

@interface RenderBrush : NSObject {
    @private
    Brush* brush;
    VBOBuffer* faceVBO;
    VBOBuffer* edgeVBO;
    VBOMemBlock* faceBlock;
    VBOMemBlock* edgeBlock;
    NSMutableDictionary* faceEntries;
    int wireframeCount;
    int wireframeIndex;
}

- (id)initWithBrush:(Brush *)theBrush faceVBO:(VBOBuffer *)theFaceVBO edgeVBO:(VBOBuffer *)theEdgeVBO;

- (Brush *)brush;

- (void)prepareFacesWithTextureManager:(TextureManager *)theTextureManager;
- (void)prepareWireframe;

- (void)indexForFace:(Face *)face indexBuffer:(IntData *)theIndexBuffer countBuffer:(IntData *)theCountBuffer;
- (void)wireFrameIndices:(IntData *)theIndexBuffer countBuffer:(IntData *)theCountBuffer;
@end

//
//  RenderContext3D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RenderContext.h"

@class Camera;
@class RenderMap;
@class TextureManager;
@class VBOBuffer;
@class SelectionManager;

@interface RenderContext3D : NSObject <RenderContext> {
    @private 
    Camera* camera;
    RenderMap* renderMap;
    VBOBuffer* faceVBO;
    VBOBuffer* edgeVBO;
    TextureManager* textureManager;
    SelectionManager* selectionManager;
}

- (id)initWithRenderMap:(RenderMap *)theRenderMap camera:(Camera *)theCamera textureManager:(TextureManager *)theTextureManager faceVBO:(VBOBuffer *)theFaceVBO edgeVBO:(VBOBuffer *)theEdgeVBO selectionManager:(SelectionManager *)theSelectionManager;

@end

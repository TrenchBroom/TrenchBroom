//
//  MapWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MapView2D;
@class MapView3D;
@class TextureView;
@class Camera;
@class RenderMap;
@class TextureManager;
@class InputManager;
@class VBOBuffer;
@class Octree;
@class Picker;
@class SelectionManager;
@class GLFontManager;

@interface MapWindowController : NSWindowController {
	IBOutlet MapView3D* view3D;
    RenderMap* renderMap;
    Camera* camera;
    TextureManager* textureManager;
    GLFontManager* fontManager;
    InputManager* inputManager;
    VBOBuffer* faceVBO;
    Octree* octree;
    Picker* picker;
    SelectionManager* selectionManager;
}

@end

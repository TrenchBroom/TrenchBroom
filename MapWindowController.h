//
//  MapWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MapView3D;
@class TextureView;
@class Camera;
@class TextureManager;
@class InputManager;
@class VBOBuffer;
@class Octree;
@class Picker;
@class SelectionManager;
@class GLFontManager;
@class SingleTextureView;
@class ToolManager;
@class Options;

@interface MapWindowController : NSWindowController {
	IBOutlet MapView3D* view3D;
    Camera* camera;
    TextureManager* textureManager;
    GLFontManager* fontManager;
    InputManager* inputManager;
    VBOBuffer* vbo;
    Octree* octree;
    Picker* picker;
    SelectionManager* selectionManager;
    ToolManager* toolManager;
    Options* options;
}

- (IBAction)toggleGrid:(id)sender;
- (IBAction)setGridSize:(id)sender;

- (VBOBuffer *)vbo;
- (Camera *)camera;
- (SelectionManager *)selectionManager;
- (InputManager *)inputManager;
- (TextureManager *)textureManager;
- (ToolManager *)toolManager;
- (Options *)options;

@end

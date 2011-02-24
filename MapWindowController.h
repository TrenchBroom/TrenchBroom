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
    Octree* octree;
    Picker* picker;
    SelectionManager* selectionManager;
    ToolManager* toolManager;
    TextureManager* textureManager;
    GLFontManager* fontManager;
    InputManager* inputManager;
    VBOBuffer* vbo;
    Options* options;
}

- (IBAction)toggleGrid:(id)sender;
- (IBAction)toggleSnap:(id)sender;
- (IBAction)setGridSize:(id)sender;

- (IBAction)clearSelection:(id)sender;
- (IBAction)copySelection:(id)sender;
- (IBAction)cutSelection:(id)sender;
- (IBAction)pasteClipboard:(id)sender;
- (IBAction)deleteSelection:(id)sender;

- (IBAction)moveTextureLeft:(id)sender;
- (IBAction)moveTextureRight:(id)sender;
- (IBAction)moveTextureUp:(id)sender;
- (IBAction)moveTextureDown:(id)sender;
- (IBAction)stretchTextureHorizontally:(id)sender;
- (IBAction)shrinkTextureHorizontally:(id)sender;
- (IBAction)stretchTextureVertically:(id)sender;
- (IBAction)shrinkTextureVertically:(id)sender;
- (IBAction)rotateTextureLeft:(id)sender;
- (IBAction)rotateTextureRight:(id)sender;

- (VBOBuffer *)vbo;
- (Camera *)camera;
- (SelectionManager *)selectionManager;
- (InputManager *)inputManager;
- (TextureManager *)textureManager;
- (ToolManager *)toolManager;
- (Options *)options;

@end

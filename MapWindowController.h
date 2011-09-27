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
@class SelectionManager;
@class GLFontManager;
@class SingleTextureView;
@class Options;
@class Prefab;
@class CursorManager;
@class ClipTool;
@class Renderer;
@class InspectorViewController;
@class ConsoleWindowController;
@class QuickBarWindowController;
@class PointFileFeedbackFigure;
@protocol Prefab;
@protocol Entity;
@protocol Brush;
@protocol Face;

@interface MapWindowController : NSWindowController <NSWindowDelegate> {
    IBOutlet NSSplitView* splitView;
	IBOutlet MapView3D* view3D;
    InspectorViewController* inspectorViewController;
    Camera* camera;
    InputManager* inputManager;
    CursorManager* cursorManager;
    Options* options;
    ConsoleWindowController* console;
    QuickBarWindowController* quickBar;
    BOOL view3DWasFirstResponder;
    PointFileFeedbackFigure* pointFileFigure;
}

- (IBAction)loadPointFile:(id)sender;
- (IBAction)unloadPointFile:(id)sender;

- (IBAction)showInspector:(id)sender;
- (IBAction)toggleGrid:(id)sender;
- (IBAction)toggleSnap:(id)sender;
- (IBAction)setGridSize:(id)sender;
- (IBAction)isolateSelection:(id)sender;
- (IBAction)toggleProjection:(id)sender;
- (IBAction)switchToXYView:(id)sender;
- (IBAction)switchToXZView:(id)sender;
- (IBAction)switchToYZView:(id)sender;

- (IBAction)stretchTextureHorizontally:(id)sender;
- (IBAction)shrinkTextureHorizontally:(id)sender;
- (IBAction)stretchTextureVertically:(id)sender;
- (IBAction)shrinkTextureVertically:(id)sender;
- (IBAction)rotateTextureLeft:(id)sender;
- (IBAction)rotateTextureRight:(id)sender;

- (IBAction)createPointEntity:(id)sender;
- (IBAction)createBrushEntity:(id)sender;

- (IBAction)rotate90CW:(id)sender;
- (IBAction)rotate90CCW:(id)sender;
- (IBAction)flipHorizontally:(id)sender;
- (IBAction)flipVertically:(id)sender;
- (IBAction)toggleClipTool:(id)sender;
- (IBAction)toggleClipMode:(id)sender;
- (IBAction)performClip:(id)sender;

- (IBAction)selectAll:(id)sender;
- (IBAction)selectNone:(id)sender;
- (IBAction)selectEntity:(id)sender;
- (IBAction)selectAllTouchingBrush:(id)sender;
- (IBAction)deleteSelection:(id)sender;
- (IBAction)duplicateSelection:(id)sender;
- (IBAction)createPrefabFromSelection:(id)sender;
- (void)insertPrefab:(id <Prefab>)prefab;

- (IBAction)compile:(id)sender;
- (IBAction)run:(id)sender;
- (IBAction)compileMostRecent:(id)sender;
- (IBAction)runDefaultEngine:(id)sender;

- (void)makeEntityVisible:(id <Entity>)theEntity;
- (void)makeBrushVisible:(id <Brush>)theBrush;
- (void)makeFaceVisible:(id <Face>)theFace;

- (Camera *)camera;
- (SelectionManager *)selectionManager;
- (InputManager *)inputManager;
- (CursorManager *)cursorManager;
- (Options *)options;
- (Renderer *)renderer;
- (MapView3D *)view3D;
- (ConsoleWindowController *)console;
@end

/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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
@class ClipTool;
@class Renderer;
@class InspectorViewController;
@class ConsoleWindowController;
@class QuickBarWindowController;
@class PointFileFeedbackFigure;
@class PreferencesManager;
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
- (IBAction)toggleTextureLock:(id)sender;

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
- (Options *)options;
- (Renderer *)renderer;
- (MapView3D *)view3D;
- (ConsoleWindowController *)console;

@end

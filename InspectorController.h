//
//  FaceInspectorController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 04.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SelectionManager;
@class TextureManager;
@class TextureView;
@class SingleTextureView;
@class GLFontManager;
@class Map;
@class Texture;

@interface InspectorController : NSWindowController {
    IBOutlet NSTextField* xOffsetField;
    IBOutlet NSTextField* yOffsetField;
    IBOutlet NSTextField* xScaleField;
    IBOutlet NSTextField* yScaleField;
    IBOutlet NSTextField* rotationField;
    IBOutlet NSTextField* textureNameField;
    IBOutlet NSSearchField* textureNameFilterField;
    IBOutlet NSSegmentedControl* textureUsageFilterSC;
    IBOutlet SingleTextureView* singleTextureView;
    IBOutlet TextureView* textureView;
    Map* map;
    SelectionManager* selectionManager;
    TextureManager* textureManager;
    GLFontManager* fontManager;
}

+ (InspectorController *)sharedInspector;

- (void)switchToContext:(NSOpenGLContext *)sharedContext selectionManager:(SelectionManager *)theSelectionManager textureManager:(TextureManager *)theTextureManager fontManager:(GLFontManager *)theFontManager map:(Map *)theMap;
- (IBAction)xOffsetTextChanged:(id)sender;
- (IBAction)yOffsetTextChanged:(id)sender;
- (IBAction)xScaleTextChanged:(id)sender;
- (IBAction)yScaleTextChanged:(id)sender;
- (IBAction)rotationTextChanged:(id)sender;
- (IBAction)textureNameFilterTextChanged:(id)sender;
- (IBAction)textureUsageFilterChanged:(id)sender;

- (void)textureSelected:(Texture *)texture;
@end

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
@class SingleTextureView;

@interface FaceInspectorController : NSWindowController {
    IBOutlet NSTextField* xOffsetField;
    IBOutlet NSTextField* yOffsetField;
    IBOutlet NSTextField* xScaleField;
    IBOutlet NSTextField* yScaleField;
    IBOutlet NSTextField* rotationField;
    IBOutlet NSTextField* textureNameField;
    IBOutlet SingleTextureView* textureView;
    SelectionManager* selectionManager;
    TextureManager* textureManager;
}

+ (FaceInspectorController *)sharedInspector;

- (void)switchToContext:(NSOpenGLContext *)sharedContext selectionManager:(SelectionManager *)theSelectionManager textureManager:(TextureManager *)theTextureManager;
- (IBAction)xOffsetTextChanged:(id)sender;
- (IBAction)yOffsetTextChanged:(id)sender;
- (IBAction)xScaleTextChanged:(id)sender;
- (IBAction)yScaleTextChanged:(id)sender;
- (IBAction)rotationTextChanged:(id)sender;

@end

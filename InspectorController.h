//
//  FaceInspectorController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 04.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MapWindowController;
@class TextureView;
@class SingleTextureView;
@class PrefabView;
@class Prefab;
@class Texture;
@class EntityPropertyTableDataSource;
@class EntityView;
@protocol Prefab;

@interface InspectorController : NSWindowController <NSTableViewDelegate> {
    IBOutlet NSTextField* xOffsetField;
    IBOutlet NSTextField* yOffsetField;
    IBOutlet NSTextField* xScaleField;
    IBOutlet NSTextField* yScaleField;
    IBOutlet NSTextField* rotationField;
    IBOutlet NSTextField* textureNameField;
    IBOutlet NSSearchField* textureNameFilterField;
    IBOutlet NSSegmentedControl* textureUsageFilterSC;
    IBOutlet NSSegmentedControl* textureSortCriterionSC;
    IBOutlet SingleTextureView* singleTextureView;
    IBOutlet TextureView* textureView;
    IBOutlet NSScrollView* textureScrollView;
    IBOutlet NSTableView* wadTableView;
    IBOutlet NSArrayController* wadArrayController;

    IBOutlet NSSlider* prefabsPerRowSlider;
    IBOutlet PrefabView* prefabView;
    IBOutlet NSBox* textureControlBox;
    
    IBOutlet NSTableView* entityPropertyTableView;
    IBOutlet NSButton* removeEntityPropertyButton;
    IBOutlet NSButton* addEntityPropertyButton;
    EntityPropertyTableDataSource* entityPropertyTableDataSource;
    IBOutlet EntityView* entityView;
    
    MapWindowController* mapWindowController;
}

+ (InspectorController *)sharedInspector;

- (void)setMapWindowController:(MapWindowController *)theMapWindowController;
- (MapWindowController *)mapWindowController;

- (IBAction)xOffsetTextChanged:(id)sender;
- (IBAction)yOffsetTextChanged:(id)sender;
- (IBAction)xScaleTextChanged:(id)sender;
- (IBAction)yScaleTextChanged:(id)sender;
- (IBAction)rotationTextChanged:(id)sender;
- (IBAction)textureNameFilterTextChanged:(id)sender;
- (IBAction)textureUsageFilterChanged:(id)sender;
- (IBAction)textureSortCriterionChanged:(id)sender;
- (IBAction)prefabsPerRowChanged:(id)sender;
- (IBAction)addTextureWad:(id)sender;
- (IBAction)toggleTextureControls:(id)sender;

- (void)textureSelected:(Texture *)texture;
- (void)prefabSelected:(id <Prefab>)prefab;

- (IBAction)removeEntityProperty:(id)sender;
- (IBAction)addEntityProperty:(id)sender;
@end

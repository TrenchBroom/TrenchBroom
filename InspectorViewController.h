/*
Copyright (C) 2010-2011 Kristian Duske

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
#import "PrefabViewTarget.h"
#import "TextureViewTarget.h"
#import "EntityDefinitionViewTarget.h"

@class MapWindowController;
@class SingleTextureView;
@class TextureView;
@class PrefabView;
@class EntityPropertyTableDataSource;
@class EntityView;
@class Texture;
@class MapBrowserDataSource;
@class GroupTableDataSource;
@protocol Prefab;

@interface InspectorViewController : NSViewController <NSTableViewDelegate, NSSplitViewDelegate, PrefabViewTarget, TextureViewTarget, EntityDefinitionViewTarget> {
    IBOutlet NSTabView* tabView;
    IBOutlet NSTextField* xOffsetField;
    IBOutlet NSStepper* xOffsetStepper;
    IBOutlet NSTextField* yOffsetField;
    IBOutlet NSStepper* yOffsetStepper;
    IBOutlet NSTextField* xScaleField;
    IBOutlet NSStepper* xScaleStepper;
    IBOutlet NSTextField* yScaleField;
    IBOutlet NSStepper* yScaleStepper;
    IBOutlet NSTextField* rotationField;
    IBOutlet NSStepper* rotationStepper;
    IBOutlet NSButton* resetFaceButton;
    IBOutlet NSTextField* textureNameField;
    IBOutlet NSSearchField* textureNameFilterField;
    IBOutlet NSSegmentedControl* textureUsageFilterSC;
    IBOutlet NSSegmentedControl* textureSortCriterionSC;
    IBOutlet SingleTextureView* singleTextureView;
    IBOutlet TextureView* textureView;
    IBOutlet NSScrollView* textureScrollView;
    IBOutlet NSTableView* wadTableView;
    IBOutlet NSArrayController* wadArrayController;
    
    IBOutlet PrefabView* prefabView;
    IBOutlet NSSlider* prefabsPerRowSlider;
    IBOutlet NSButton* addPrefabButton;
    IBOutlet NSButton* editPrefabButton;
    IBOutlet NSButton* removePrefabButton;
    
    IBOutlet NSTableView* entityPropertyTableView;
    IBOutlet NSButton* removeEntityPropertyButton;
    IBOutlet NSButton* addEntityPropertyButton;
    EntityPropertyTableDataSource* entityPropertyTableDataSource;
    IBOutlet EntityView* entityView;
    IBOutlet NSSegmentedControl* entitySortCriterionSC;
    IBOutlet NSSegmentedControl* entityUsageFilterSC;
    IBOutlet NSSearchField* entityNameFilterField;
    
    IBOutlet NSOutlineView* mapBrowserView;
    MapBrowserDataSource* mapBrowserDataSource;
    
    IBOutlet NSTableView* groupTableView;
    GroupTableDataSource* groupTableDataSource;
    IBOutlet NSButton* addGroupButton;
    IBOutlet NSButton* removeGroupButton;
    
    MapWindowController* mapWindowController;
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController;
- (MapWindowController *)mapWindowController;

- (IBAction)xOffsetTextChanged:(id)sender;
- (IBAction)xOffsetStepperChanged:(id)sender;
- (IBAction)yOffsetTextChanged:(id)sender;
- (IBAction)yOffsetStepperChanged:(id)sender;
- (IBAction)xScaleTextChanged:(id)sender;
- (IBAction)xScaleStepperChanged:(id)sender;
- (IBAction)yScaleTextChanged:(id)sender;
- (IBAction)yScaleStepperChanged:(id)sender;
- (IBAction)rotationTextChanged:(id)sender;
- (IBAction)rotationStepperChanged:(id)sender;
- (IBAction)resetFace:(id)sender;
- (IBAction)textureNameFilterTextChanged:(id)sender;
- (IBAction)textureUsageFilterChanged:(id)sender;
- (IBAction)textureSortCriterionChanged:(id)sender;
- (IBAction)prefabsPerRowChanged:(id)sender;
- (IBAction)addTextureWad:(id)sender;

- (IBAction)addPrefab:(id)sender;
- (IBAction)editPrefab:(id)sender;
- (IBAction)removePrefab:(id)sender;

- (IBAction)removeEntityProperty:(id)sender;
- (IBAction)addEntityProperty:(id)sender;
- (IBAction)entityNameFilterTextChanged:(id)sender;
- (IBAction)entityUsageFilterChanged:(id)sender;
- (IBAction)entitySortCriterionChanged:(id)sender;

- (IBAction)mapBrowserClicked:(id)sender;

- (IBAction)addGroup:(id)sender;
- (IBAction)removeGroup:(id)sender;
@end

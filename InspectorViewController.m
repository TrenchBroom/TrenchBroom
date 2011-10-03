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

#import "InspectorViewController.h"
#import "MapDocument.h"
#import "Prefab.h"
#import "PrefabGroup.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "GLResources.h"
#import "MapWindowController.h"
#import "SelectionManager.h"
#import "TextureManager.h"
#import "TextureFilter.h"
#import "TextureNameFilter.h"
#import "TextureUsageFilter.h"
#import "EntityDefinitionFilter.h"
#import "EntityDefinitionNameFilter.h"
#import "EntityDefinitionUsageFilter.h"
#import "SingleTextureView.h"
#import "TextureView.h"
#import "PrefabView.h"
#import "EntityView.h"
#import "EntityPropertyTableDataSource.h"
#import "MapBrowserDataSource.h"
#import "GroupTableDataSource.h"
#import "ControllerUtils.h"
#import "PreferencesManager.h"
#import "PrefabNameSheetController.h"
#import "PrefabManager.h"
#import "GroupManager.h"

@interface InspectorViewController (private)

- (void)groupsDidChange:(NSNotification *)notification;
- (void)preferencesDidChange:(NSNotification *)notification;
- (void)entityCountChanged:(NSNotification *)notification;
- (void)propertiesDidChange:(NSNotification *)notification;
- (void)brushCountChanged:(NSNotification *)notification;
- (void)brushesDidChange:(NSNotification *)notification;
- (void)facesDidChange:(NSNotification *)notification;
- (void)selectionChanged:(NSNotification *)notification;
- (void)prefabSelectionDidChange:(NSNotification *)notification;
- (void)textureManagerChanged:(NSNotification *)notification;
- (void)updateMapWindowController:(MapWindowController *)theMapWindowController;
- (void)updateTextureControls;
- (void)updateTextureFilter;
- (void)updateEntityDefinitionFilter;
- (void)updateEntityPropertyTable;
- (void)prefabNameSheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;

@end

@implementation InspectorViewController (private)

- (void)groupsDidChange:(NSNotification *)notification {
    [groupTableView reloadData];
}

- (void)preferencesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    if (DefaultsQuakePath != [userInfo objectForKey:DefaultsKey])
        return;
    
    [entityView setNeedsDisplay:YES];
}

- (void)entityCountChanged:(NSNotification *)notification {
    [mapBrowserView reloadData];
}

- (void)propertiesDidChange:(NSNotification *)notification {
    [entityPropertyTableDataSource updateProperties];
    [entityPropertyTableView reloadData];
    [mapBrowserView reloadData];
    
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if ([entity isWorldspawn]) {
            [entityView setMods:modListFromWorldspawn(entity)];
            break;
        }
    }
}

- (void)brushCountChanged:(NSNotification *)notification {
    [mapBrowserView reloadData];
}

- (void)brushesDidChange:(NSNotification *)notification {
    [self updateTextureControls];
    [mapBrowserView reloadData];
}

- (void)facesDidChange:(NSNotification *)notification {
    [self updateTextureControls];
    [mapBrowserView reloadData];
}

- (void)selectionChanged:(NSNotification *)notification {
    [self updateEntityPropertyTable];
    [self updateTextureControls];
    
    SelectionManager* selectionManager = [[mapWindowController document] selectionManager];
    [addPrefabButton setEnabled:[selectionManager hasSelectedBrushes]];
    [addGroupButton setEnabled:[selectionManager hasSelectedBrushes]];
}

- (void)prefabSelectionDidChange:(NSNotification *)notification {
    id <Prefab> prefab = [prefabView selectedPrefab];
    [editPrefabButton setEnabled:prefab != nil && ![prefab readOnly]];
    [removePrefabButton setEnabled:prefab != nil && ![prefab readOnly]];
}

- (void)textureManagerChanged:(NSNotification *)notification {
    [wadTableView reloadData];
}

- (void)updateMapWindowController:(MapWindowController *)theMapWindowController {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    if (mapWindowController != nil) {
        MapDocument* map = [mapWindowController document];
        GLResources* glResources = [map glResources];
        
        TextureManager* textureManager = [glResources textureManager];
        SelectionManager* selectionManager = [map selectionManager];
        GroupManager* groupManager = [map groupManager];
        
        [center removeObserver:self name:SelectionAdded object:selectionManager];
        [center removeObserver:self name:SelectionRemoved object:selectionManager];
        [center removeObserver:self name:EntitiesAdded object:map];
        [center removeObserver:self name:EntitiesWereRemoved object:map];
        [center removeObserver:self name:PropertiesDidChange object:map];
        [center removeObserver:self name:BrushesAdded object:map];
        [center removeObserver:self name:BrushesWereRemoved object:map];
        [center removeObserver:self name:BrushesDidChange object:map];
        [center removeObserver:self name:FacesDidChange object:map];
        [center removeObserver:self name:TextureManagerChanged object:textureManager];
        [center removeObserver:self name:GroupsChanged object:groupManager];
        
        [entityPropertyTableDataSource setMapWindowController:nil];
        [mapBrowserDataSource setMapWindowController:nil];
        [groupTableDataSource setMapWindowController:nil];
    }
    
    mapWindowController = theMapWindowController;
    
    if (mapWindowController != nil) {
        MapDocument* map = [mapWindowController document];
        SelectionManager* selectionManager = [map selectionManager];
        GroupManager* groupManager = [map groupManager];

        GLResources* glResources = [map glResources];
        EntityDefinitionManager* entityDefinitionManager = [map entityDefinitionManager];
        
        [singleTextureView setGLResources:glResources];
        [textureView setGLResources:glResources];
        [textureView setSelectionManager:selectionManager];
        [prefabView setGLResources:glResources];
        [entityView setGLResources:glResources entityDefinitionManager:entityDefinitionManager];
        [entityView setMods:modListFromWorldspawn([map worldspawn:YES])];
        
        TextureManager* textureManager = [glResources textureManager];
        [entityPropertyTableDataSource setMapWindowController:mapWindowController];
        [mapBrowserDataSource setMapWindowController:mapWindowController];
        [groupTableDataSource setMapWindowController:mapWindowController];
        
        [center addObserver:self selector:@selector(selectionChanged:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionChanged:) name:SelectionRemoved object:selectionManager];
        [center addObserver:self selector:@selector(entityCountChanged:) name:EntitiesAdded object:map];
        [center addObserver:self selector:@selector(entityCountChanged:) name:EntitiesWereRemoved object:map];
        [center addObserver:self selector:@selector(propertiesDidChange:) name:PropertiesDidChange object:map];
        [center addObserver:self selector:@selector(brushCountChanged:) name:BrushesAdded object:map];
        [center addObserver:self selector:@selector(brushCountChanged:) name:BrushesWereRemoved object:map];
        [center addObserver:self selector:@selector(brushesDidChange:) name:BrushesDidChange object:map];
        [center addObserver:self selector:@selector(facesDidChange:) name:FacesDidChange object:map];
        [center addObserver:self selector:@selector(textureManagerChanged:) name:TextureManagerChanged object:textureManager];
        [center addObserver:self selector:@selector(groupsDidChange:) name:GroupsChanged object:groupManager];

        [addPrefabButton setEnabled:[selectionManager hasSelectedBrushes]];
        [addGroupButton setEnabled:[selectionManager hasSelectedBrushes]];
        [removeGroupButton setEnabled:[groupTableView selectedRow] != -1];
    } else {
        [singleTextureView setGLResources:nil];
        [textureView setGLResources:nil];
        [textureView setSelectionManager:nil];
        [prefabView setGLResources:nil];
        [entityView setGLResources:nil entityDefinitionManager:nil];
        [entityView setMods:nil];

        [addPrefabButton setEnabled:NO];
        [addGroupButton setEnabled:NO];
        [removeGroupButton setEnabled:NO];
    }

    [mapBrowserView reloadData];
    [self updateEntityPropertyTable];
    [self updateTextureControls];
    [wadTableView reloadData];
    [groupTableView reloadData];
}

- (void)updateTextureControls {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSArray* selectedFaces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    
    if ([selectedFaces count] > 0) {
        [xOffsetField setEnabled:YES];
        [yOffsetField setEnabled:YES];
        [xScaleField setEnabled:YES];
        [yScaleField setEnabled:YES];
        [rotationField setEnabled:YES];
        
        NSEnumerator* faceEn = [selectedFaces objectEnumerator];
        id <Face> face = [faceEn nextObject];
        
        int xOffset = [face xOffset];
        int yOffset = [face yOffset];
        float xScale = [face xScale];
        float yScale = [face yScale];
        float rotation = [face rotation];
        NSString* textureName = [face texture];
        
        BOOL xOffsetMultiple = NO;
        BOOL yOffsetMultiple = NO;
        BOOL xScaleMultiple = NO;
        BOOL yScaleMultiple = NO;
        BOOL rotationMultiple = NO;
        BOOL textureMultiple = NO;
        
        while ((face = [faceEn nextObject])) {
            xOffsetMultiple  |= xOffset  != [face xOffset];
            yOffsetMultiple  |= yOffset  != [face yOffset];
            xScaleMultiple   |= xScale   != [face xScale];
            yScaleMultiple   |= yScale   != [face yScale];
            rotationMultiple |= rotation != [face rotation];
            textureMultiple  |= ![textureName isEqualToString:[face texture]];
        }
        
        [xOffsetStepper setEnabled:!xOffsetMultiple];
        if (xOffsetMultiple) {
            [[xOffsetField cell] setPlaceholderString:@"multiple"];
            [xOffsetField setStringValue:@""];
        } else {
            [xOffsetField setIntValue:xOffset];
            [xOffsetStepper setIntValue:xOffset];
        }
        
        [yOffsetStepper setEnabled:!yOffsetMultiple];
        if (yOffsetMultiple) {
            [[yOffsetField cell] setPlaceholderString:@"multiple"];
            [yOffsetField setStringValue:@""];
        } else {
            [yOffsetField setIntValue:yOffset];
            [yOffsetStepper setIntValue:yOffset];
        }
        
        [xScaleStepper setEnabled:!xScaleMultiple];
        if (xScaleMultiple) {
            [[xScaleField cell] setPlaceholderString:@"multiple"];
            [xScaleField setStringValue:@""];
        } else {
            [xScaleField setFloatValue:xScale];
            [xScaleStepper setFloatValue:xScale];
        }
        
        [yScaleStepper setEnabled:!yScaleMultiple];
        if (yScaleMultiple) {
            [[yScaleField cell] setPlaceholderString:@"multiple"];
            [yScaleField setStringValue:@""];
        } else {
            [yScaleField setFloatValue:yScale];
            [yScaleStepper setFloatValue:yScale];
        }
        
        [rotationStepper setEnabled:!rotationMultiple];
        if (rotationMultiple) {
            [[rotationField cell] setPlaceholderString:@"multiple"];
            [rotationField setStringValue:@""];
        } else {
            [rotationField setFloatValue:rotation];
            [rotationStepper setFloatValue:rotation];
        }
        
        if (textureMultiple) {
            [[textureNameField cell] setPlaceholderString:@"multiple"];
            [textureNameField setStringValue:@""];
            [singleTextureView setTextureName:nil];
        } else {
            [textureNameField setStringValue:textureName];
            [singleTextureView setTextureName:textureName];
        }
    } else {
        [xOffsetField setEnabled:NO];
        [xOffsetStepper setEnabled:NO];
        [yOffsetField setEnabled:NO];
        [yOffsetStepper setEnabled:NO];
        [xScaleField setEnabled:NO];
        [xScaleStepper setEnabled:NO];
        [yScaleField setEnabled:NO];
        [yScaleStepper setEnabled:NO];
        [rotationField setEnabled:NO];
        [rotationStepper setEnabled:NO];
        
        [[xOffsetField cell] setPlaceholderString:@"n/a"];
        [[yOffsetField cell] setPlaceholderString:@"n/a"];
        [[xScaleField cell] setPlaceholderString:@"n/a"];
        [[yScaleField cell] setPlaceholderString:@"n/a"];
        [[rotationField cell] setPlaceholderString:@"n/a"];
        [[textureNameField cell] setPlaceholderString:@"n/a"];
        
        [xOffsetField setStringValue:@""];
        [yOffsetField setStringValue:@""];
        [xScaleField setStringValue:@""];
        [yScaleField setStringValue:@""];
        [rotationField setStringValue:@""];
        [textureNameField setStringValue:@""];
        [singleTextureView setTextureName:nil];
    }
}

- (void)updateTextureFilter {
    id<TextureFilter> filter = nil;
    NSString* pattern = [textureNameFilterField stringValue];
    
    if (pattern != nil && [pattern length] > 0)
        filter = [[TextureNameFilter alloc] initWithPattern:pattern];
    
    if ([textureUsageFilterSC selectedSegment] == 1) {
        id<TextureFilter> temp = [[TextureUsageFilter alloc] initWithFilter:filter];
        [filter release];
        filter = temp;
    }
    
    [textureView setTextureFilter:filter];
    [filter release];
}

- (void)updateEntityDefinitionFilter {
    id <EntityDefinitionFilter> filter = nil;
    NSString* pattern = [entityNameFilterField stringValue];
    
    if (pattern != nil && [pattern length] > 0)
        filter = [[EntityDefinitionNameFilter alloc] initWithPattern:pattern];
    
    if ([entityUsageFilterSC selectedSegment] == 1) {
        id <EntityDefinitionFilter> temp = [[EntityDefinitionUsageFilter alloc] initWithFilter:filter];
        [filter release];
        filter = temp;
    }
    
    [entityView setEntityDefinitionFilter:filter];
    [filter release];
}

- (void)updateEntityPropertyTable {
    if (mapWindowController != nil) {
        SelectionManager* selectionManager = [mapWindowController selectionManager];
        NSArray* selectedEntities = [selectionManager selectedEntities];
        if ([selectedEntities count] == 0) {
            id <Map> map = [mapWindowController document];
            id <Entity> worldspawn = [map worldspawn:YES];
            [entityPropertyTableDataSource setEntities:[NSArray arrayWithObject:worldspawn]];
        } else {
            [entityPropertyTableDataSource setEntities:selectedEntities];
        }
    } else {
        [entityPropertyTableDataSource setEntities:nil];
    }

    [entityPropertyTableView reloadData];
}

- (void)prefabNameSheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo {
    PrefabNameSheetController* pns = [sheet windowController];
    if (returnCode == NSOKButton) {
        id <Prefab> prefab = [prefabView selectedPrefab];
        if (prefab != nil) {
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
            NSString* prefabName = [pns prefabName];
            NSString* prefabGroupName = [pns prefabGroup];
            [prefabManager renamePrefab:prefab newName:prefabName newPrefabGroupName:prefabGroupName];
        }
    }
    [pns release];
}

@end

@implementation InspectorViewController

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [entityPropertyTableDataSource release];
    [mapBrowserDataSource release];
    [groupTableDataSource release];
    [super dealloc];
}

- (void)loadView {
    [super loadView];
    
    [xOffsetStepper setMinValue:INT16_MIN];
    [xOffsetStepper setMaxValue:INT16_MAX];
    [xOffsetStepper setIncrement:1];

    [yOffsetStepper setMinValue:INT16_MIN];
    [yOffsetStepper setMaxValue:INT16_MAX];
    [yOffsetStepper setIncrement:1];
    
    [xScaleStepper setMinValue:-FLT_MAX];
    [xScaleStepper setMaxValue:FLT_MAX];
    [xScaleStepper setIncrement:0.05];

    [yScaleStepper setMinValue:-FLT_MAX];
    [yScaleStepper setMaxValue:FLT_MAX];
    [yScaleStepper setIncrement:0.05];
    
    [rotationStepper setMinValue:-FLT_MAX];
    [rotationStepper setMaxValue:FLT_MAX];
    [rotationStepper setIncrement:1];

    entityPropertyTableDataSource = [[EntityPropertyTableDataSource alloc] init];
    [entityPropertyTableView setDataSource:entityPropertyTableDataSource];
    
    mapBrowserDataSource = [[MapBrowserDataSource alloc] init];
    [mapBrowserView setDataSource:mapBrowserDataSource];
    
    groupTableDataSource = [[GroupTableDataSource alloc] init];
    [groupTableView setDataSource:groupTableDataSource];
    
    [self updateMapWindowController:mapWindowController];
    [self prefabsPerRowChanged:prefabsPerRowSlider];
    
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:preferences];
    [center addObserver:self selector:@selector(prefabSelectionDidChange:) name:PrefabSelectionDidChange object:prefabView];
    
    [tabView selectTabViewItemAtIndex:0];
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController {
    if (mapWindowController == theMapWindowController)
        return;
    
    [self updateMapWindowController:theMapWindowController];
}

- (MapWindowController *)mapWindowController {
    return mapWindowController;
}

#pragma mark @implementation NSTableViewDelegate

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
    if ([notification object] == entityPropertyTableView) {
        NSIndexSet* selectedRows = [entityPropertyTableView selectedRowIndexes];
        if ([selectedRows count] > 0) {
            MapDocument* map = [mapWindowController document];
            SelectionManager* selectionManager = [map selectionManager];
            NSArray* entities = [selectionManager selectedEntities];
            
            if ([entities count] == 0)
                entities = [NSArray arrayWithObject:[map worldspawn:YES]];
            
            NSUInteger index = [selectedRows firstIndex];
            do {
                NSString* key = [entityPropertyTableDataSource propertyKeyAtIndex:index];
                NSEnumerator* entityEn = [entities objectEnumerator];
                id <Entity> entity;
                while ((entity = [entityEn nextObject])) {
                    if (![entity isPropertyDeletable:key]) {
                        [removeEntityPropertyButton setEnabled:NO];
                        return;
                    }
                }
            } while ((index = [selectedRows indexGreaterThanIndex:index]) != NSNotFound);
            
            [removeEntityPropertyButton setEnabled:YES];
        } else {
            [removeEntityPropertyButton setEnabled:NO];
        }
    } else if ([notification object] == groupTableView) {
        [removeGroupButton setEnabled:[groupTableView selectedRow] != -1];
    }
}

- (BOOL)tableView:(NSTableView *)theTableView shouldEditTableColumn:(NSTableColumn *)theTableColumn row:(NSInteger)theRowIndex {
    if (theTableView == entityPropertyTableView)
        return [entityPropertyTableDataSource editingAllowed:theTableColumn rowIndex:theRowIndex];
    return YES;
}

#pragma mark @implementation NSSplitViewDelegate

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMinimumPosition ofSubviewAt:(NSInteger)dividerIndex {
    return 150;
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex {
    float height = NSHeight([splitView frame]);
    return height - 150;
}

#pragma mark Texture controls

- (IBAction)xOffsetTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    int xOffset = [xOffsetField intValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces xOffset:xOffset];
    
    [undoManager setActionName:@"Set Texture X Offset"];
    [undoManager endUndoGrouping];
}

- (IBAction)xOffsetStepperChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    int xOffset = [xOffsetStepper intValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces xOffset:xOffset];
    
    [undoManager setActionName:@"Set Texture X Offset"];
    [undoManager endUndoGrouping];
}

- (IBAction)yOffsetTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    int yOffset = [yOffsetField intValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces yOffset:yOffset];
    
    [undoManager setActionName:@"Set Texture Y Offset"];
    [undoManager endUndoGrouping];
}

- (IBAction)yOffsetStepperChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    int yOffset = [yOffsetStepper intValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces yOffset:yOffset];
    
    [undoManager setActionName:@"Set Texture Y Offset"];
    [undoManager endUndoGrouping];
}

- (IBAction)xScaleTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float xScale = [xScaleField floatValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces xScale:xScale];
    
    [undoManager setActionName:@"Set Texture X Scale"];
    [undoManager endUndoGrouping];
}

- (IBAction)xScaleStepperChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float xScale = [xScaleStepper floatValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces xScale:xScale];
    
    [undoManager setActionName:@"Set Texture X Scale"];
    [undoManager endUndoGrouping];
}

- (IBAction)yScaleTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float yScale = [yScaleField floatValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces yScale:yScale];
    
    [undoManager setActionName:@"Set Texture Y Scale"];
    [undoManager endUndoGrouping];
}

- (IBAction)yScaleStepperChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float yScale = [yScaleStepper floatValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces yScale:yScale];
    
    [undoManager setActionName:@"Set Texture Y Scale"];
    [undoManager endUndoGrouping];
}

- (IBAction)rotationTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float rotation = [rotationField floatValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces rotation:rotation];
    
    [undoManager setActionName:@"Set Texture Rotation"];
    [undoManager endUndoGrouping];
}

- (IBAction)rotationStepperChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float rotation = [rotationStepper floatValue];
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces rotation:rotation];
    
    [undoManager setActionName:@"Set Texture Rotation"];
    [undoManager endUndoGrouping];
}

- (void)textureSelected:(Texture *)texture {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    NSArray* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces texture:[texture name]];
    
    [undoManager setActionName:@"Set Texture"];
    [undoManager endUndoGrouping];
}

- (IBAction)textureNameFilterTextChanged:(id)sender {
    [self updateTextureFilter];
}

- (IBAction)textureUsageFilterChanged:(id)sender {
    [self updateTextureFilter];
}

- (IBAction)textureSortCriterionChanged:(id)sender {
    switch ([textureSortCriterionSC selectedSegment]) {
        case 0:
            [textureView setSortCriterion:TS_NAME];
            break;
        default:
            [textureView setSortCriterion:TS_USAGE];
            break;
    }
}

- (IBAction)addTextureWad:(id)sender {
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setAllowsMultipleSelection:YES];
    [openPanel setAllowedFileTypes:[NSArray arrayWithObject:@"wad"]];
    [openPanel setAllowsOtherFileTypes:NO];
    [openPanel setTitle:@"Choose Wad File(s)"];
    [openPanel setNameFieldLabel:@"Wad File"];
    [openPanel setCanCreateDirectories:NO];
    
    if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
        NSEnumerator* urlEn = [[openPanel URLs] objectEnumerator];
        NSURL* url;
        while ((url = [urlEn nextObject])) {
            NSString* wadPath = [url path];
            if (wadPath != nil)
                [wadArrayController addObject:wadPath];
        }
    }
}

#pragma mark Prefab controls

- (IBAction)prefabsPerRowChanged:(id)sender {
    [prefabView setPrefabsPerRow:[prefabsPerRowSlider intValue]];
}

- (void)prefabSelected:(id <Prefab>)prefab {
    [mapWindowController insertPrefab:prefab];
}

#pragma mark Entity controls

- (IBAction)addEntityProperty:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSArray* entities = [selectionManager selectedEntities];
    
    if ([entities count] == 0)
        entities = [NSArray arrayWithObject:[map worldspawn:YES]];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    [map setEntities:entities propertyKey:@"new_property" value:@""];
    
    [undoManager setActionName:@"Add Entity Property"];
    [undoManager endUndoGrouping];

    [entityPropertyTableView deselectAll:self];
    NSUInteger row = [entityPropertyTableDataSource indexOfPropertyWithKey:@"new_property"];
    if (row != NSNotFound)
        [entityPropertyTableView editColumn:0 row:row withEvent:nil select:YES];
}

- (IBAction)addPrefab:(id)sender {
    [mapWindowController createPrefabFromSelection:sender];
}

- (IBAction)editPrefab:(id)sender {
    id <Prefab> prefab = [prefabView selectedPrefab];
    if (prefab != nil) {
        PrefabNameSheetController* pns = [[PrefabNameSheetController alloc] init];
        NSWindow* prefabNameSheet = [pns window];
        
        [pns setPrefabName:[prefab name]];
        [pns setPrefabGroup:[[prefab prefabGroup] name]];
        
        NSApplication* app = [NSApplication sharedApplication];
        [app beginSheet:prefabNameSheet modalForWindow:[[self view] window] modalDelegate:self didEndSelector:@selector(prefabNameSheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
    }
}

- (IBAction)removePrefab:(id)sender {
    id <Prefab> prefab = [prefabView selectedPrefab];
    if (prefab != nil) {
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
        [prefabManager removePrefab:prefab];
    }
}

- (IBAction)removeEntityProperty:(id)sender {
    NSIndexSet* selectedRows = [entityPropertyTableView selectedRowIndexes];
    if ([selectedRows count] == 0)
        return;
    
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSArray* entities = [selectionManager selectedEntities];
    
    if ([entities count] == 0)
        entities = [NSArray arrayWithObject:[map worldspawn:YES]];

    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    NSUInteger index = [selectedRows firstIndex];
    do {
        NSString* key = [entityPropertyTableDataSource propertyKeyAtIndex:index];
        if (key != nil)
            [map setEntities:entities propertyKey:key value:nil];
    } while ((index = [selectedRows indexGreaterThanIndex:index]) != NSNotFound);
    
    [undoManager setActionName:@"Remove Entity Properties"];
    [undoManager endUndoGrouping];
}

- (IBAction)entityNameFilterTextChanged:(id)sender {
    [self updateEntityDefinitionFilter];
}

- (IBAction)entityUsageFilterChanged:(id)sender {
    [self updateEntityDefinitionFilter];
}

- (IBAction)entitySortCriterionChanged:(id)sender {
    switch ([entitySortCriterionSC selectedSegment]) {
        case 0:
            [entityView setSortCriterion:ES_NAME];
            break;
        default:
            [entityView setSortCriterion:ES_USAGE];
            break;
    }
}

- (void)entityDefinitionSelected:(EntityDefinition *)theDefinition {
    NSLog(@"entityDefinitionSelected:(EntityDefinition *)theDefinition not implemented");
}

#pragma mark Map browser controls

- (IBAction)mapBrowserClicked:(id)sender {
    int row = [mapBrowserView selectedRow];
    if (row > -1) {
        id item = [mapBrowserView itemAtRow:row];
        if ([item conformsToProtocol:@protocol(Map)]) {
        } else if ([item conformsToProtocol:@protocol(Entity)]) {
            id <Entity> entity = item;
            [mapWindowController makeEntityVisible:entity];
        } else if ([item conformsToProtocol:@protocol(Brush)]) {
            id <Brush> brush = item;
            [mapWindowController makeBrushVisible:brush];
        } else if ([item conformsToProtocol:@protocol(Face)]) {
            id <Face> face = item;
            [mapWindowController makeFaceVisible:face];
        }
    }
}

#pragma mark Group controls

- (IBAction)addGroup:(id)sender {
    if (mapWindowController == nil)
        return;
    
    MapDocument* map = [mapWindowController document];
    EntityDefinitionManager* entityDefinitionManager = [map entityDefinitionManager];
    SelectionManager* selectionManager = [map selectionManager];
    
    NSArray* brushes = [selectionManager selectedBrushes];
    if ([brushes count] == 0)
        return;
    
    EntityDefinition* definition = [entityDefinitionManager definitionForName:GroupClassName];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    id <Entity> entity = [map createEntityWithClassname:[definition name]];
    [map moveBrushesToEntity:entity brushes:brushes];
    [selectionManager addEntity:entity record:YES];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Create Group"];}

- (IBAction)removeGroup:(id)sender {
    if (mapWindowController == nil)
        return;
    
    NSInteger rowIndex = [groupTableView selectedRow];
    if (rowIndex == -1)
        return;
    
    MapDocument* map = [mapWindowController document];
    GroupManager* groupManager = [map groupManager];
    NSArray* groups = [groupManager groups];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    id <Entity> group = [groups objectAtIndex:rowIndex];
    NSArray* brushArray = [[NSArray alloc] initWithArray:[group brushes]];
    [map moveBrushesToEntity:[map worldspawn:YES] brushes:brushArray];
    [brushArray release];
    
    NSArray* groupArray = [[NSArray alloc] initWithObjects:group, nil];
    [map deleteEntities:groupArray];
    [groupArray release];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Remove Group"];
}

@end

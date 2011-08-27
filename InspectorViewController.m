//
//  InspectorViewController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
#import "ControllerUtils.h"
#import "PreferencesManager.h"
#import "PrefabNameSheetController.h"
#import "PrefabManager.h"

@interface InspectorViewController (private)

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
    [mapBrowserView reloadData];
}

- (void)facesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* faces = [userInfo objectForKey:FacesKey];
    
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSEnumerator* faceEn = [faces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        if ([selectionManager isFaceSelected:face])
            [self updateTextureControls];
    }

    [mapBrowserView reloadData];
}

- (void)selectionChanged:(NSNotification *)notification {
    [self updateEntityPropertyTable];
    [self updateTextureControls];
    
    SelectionManager* selectionManager = [[mapWindowController document] selectionManager];
    [addPrefabButton setEnabled:[selectionManager hasSelectedBrushes]];
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
        SelectionManager* selectionManager = [mapWindowController selectionManager];
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
        
        [entityPropertyTableDataSource setMapWindowController:nil];
        [mapBrowserDataSource setMapWindowController:nil];
    }
    
    mapWindowController = theMapWindowController;
    
    if (mapWindowController != nil) {
        MapDocument* map = [mapWindowController document];
        SelectionManager* selectionManager = [map selectionManager];
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
    } else {
        [singleTextureView setGLResources:nil];
        [textureView setGLResources:nil];
        [textureView setSelectionManager:nil];
        [prefabView setGLResources:nil];
        [entityView setGLResources:nil entityDefinitionManager:nil];
        [entityView setMods:nil];
    }

    [self updateEntityPropertyTable];
    [mapBrowserView reloadData];
    [wadTableView reloadData];
    [self updateTextureControls];
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
        
        if (xOffsetMultiple) {
            [[xOffsetField cell] setPlaceholderString:@"multiple"];
            [xOffsetField setStringValue:@""];
        } else {
            [xOffsetField setIntValue:xOffset];
        }
        
        if (yOffsetMultiple) {
            [[yOffsetField cell] setPlaceholderString:@"multiple"];
            [yOffsetField setStringValue:@""];
        } else {
            [yOffsetField setIntValue:yOffset];
        }
        
        if (xScaleMultiple) {
            [[xScaleField cell] setPlaceholderString:@"multiple"];
            [xScaleField setStringValue:@""];
        } else {
            [xScaleField setFloatValue:xScale];
        }
        
        if (yScaleMultiple) {
            [[yScaleField cell] setPlaceholderString:@"multiple"];
            [yScaleField setStringValue:@""];
        } else {
            [yScaleField setFloatValue:yScale];
        }
        
        if (rotationMultiple) {
            [[rotationField cell] setPlaceholderString:@"multiple"];
            [rotationField setStringValue:@""];
        } else {
            [rotationField setFloatValue:rotation];
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
        [yOffsetField setEnabled:NO];
        [xScaleField setEnabled:NO];
        [yScaleField setEnabled:NO];
        [rotationField setEnabled:NO];
        
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
    [super dealloc];
}

- (void)loadView {
    [super loadView];
    
    entityPropertyTableDataSource = [[EntityPropertyTableDataSource alloc] init];
    [entityPropertyTableView setDataSource:entityPropertyTableDataSource];
    
    mapBrowserDataSource = [[MapBrowserDataSource alloc] init];
    [mapBrowserView setDataSource:mapBrowserDataSource];
    
    [self updateMapWindowController:mapWindowController];
    [self prefabsPerRowChanged:prefabsPerRowSlider];
    
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:preferences];
    [center addObserver:self selector:@selector(prefabSelectionDidChange:) name:PrefabSelectionDidChange object:prefabView];
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController {
    if (mapWindowController == theMapWindowController)
        return;
    
    [self updateMapWindowController:theMapWindowController];
}

- (MapWindowController *)mapWindowController {
    return mapWindowController;
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
    }
}

- (BOOL)tableView:(NSTableView *)theTableView shouldEditTableColumn:(NSTableColumn *)theTableColumn row:(NSInteger)theRowIndex {
    if (theTableView == entityPropertyTableView)
        return [entityPropertyTableDataSource editingAllowed:theTableColumn rowIndex:theRowIndex];
    return NO;
}

- (void)entityDefinitionSelected:(EntityDefinition *)theDefinition {
    NSLog(@"entityDefinitionSelected:(EntityDefinition *)theDefinition not implemented");
}

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
@end

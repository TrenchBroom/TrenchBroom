//
//  FaceInspectorController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 04.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "InspectorController.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "GLResources.h"
#import "SelectionManager.h"
#import "TextureManager.h"
#import "TextureCollection.h"
#import "GLFontManager.h"
#import "TextureView.h"
#import "SingleTextureView.h"
#import "TextureNameFilter.h"
#import "TextureUsageFilter.h"
#import "MapDocument.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "PrefabView.h"
#import "EntityPropertyTableDataSource.h"

static InspectorController* sharedInstance = nil;

@interface InspectorController (private)

- (void)propertiesDidChange:(NSNotification *)notification;
- (void)facesDidChange:(NSNotification *)notification;
- (void)selectionRemoved:(NSNotification *)notification;
- (void)selectionAdded:(NSNotification *)notification;
- (void)textureManagerChanged:(NSNotification *)notification;
- (void)updateMapWindowController:(MapWindowController *)theMapWindowController;
- (void)updateTextureControls;
- (void)updateFilter;

@end

@implementation InspectorController (private)

- (void)propertiesDidChange:(NSNotification *)notification {
    [entityPropertyTableDataSource updateProperties];
    [entityPropertyTableView reloadData];
}

- (void)facesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* faces = [userInfo objectForKey:FacesKey];
    
    SelectionManager* selectionManager = [mapWindowController selectionManager];

    NSEnumerator* faceEn = [faces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        if ([selectionManager isFaceSelected:face])
            [self updateTextureControls];
    }
}

- (void)selectionRemoved:(NSNotification *)notification {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    [entityPropertyTableDataSource setEntities:[selectionManager selectedEntities]];
    [self updateTextureControls];
    [entityPropertyTableView reloadData];
}

- (void)selectionAdded:(NSNotification *)notification {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    [entityPropertyTableDataSource setEntities:[selectionManager selectedEntities]];
    [self updateTextureControls];
    [entityPropertyTableView reloadData];
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
        [center removeObserver:self name:PropertiesDidChange object:map];
        [center removeObserver:self name:FacesDidChange object:map];
        [center removeObserver:self name:TextureManagerChanged object:textureManager];
        
        [entityPropertyTableDataSource setMapWindowController:nil];
        [entityPropertyTableDataSource setEntities:nil];
        
        [mapWindowController release];
    }
    
    mapWindowController = [theMapWindowController retain];
    
    if (mapWindowController != nil) {
        MapDocument* map = [mapWindowController document];
        GLResources* glResources = [map glResources];
        
        [singleTextureView setGLResources:glResources];
        [textureView setGLResources:glResources];
        [prefabView setGLResources:glResources];
        
        TextureManager* textureManager = [glResources textureManager];
        SelectionManager* selectionManager = [mapWindowController selectionManager];
        [entityPropertyTableDataSource setMapWindowController:mapWindowController];
        [entityPropertyTableDataSource setEntities:[selectionManager selectedEntities]];
         
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
        [center addObserver:self selector:@selector(propertiesDidChange:) name:PropertiesDidChange object:map];
        [center addObserver:self selector:@selector(facesDidChange:) name:FacesDidChange object:map];
        [center addObserver:self selector:@selector(textureManagerChanged:) name:TextureManagerChanged object:textureManager];
    } else {
        [singleTextureView setGLResources:nil];
        [textureView setGLResources:nil];
        [prefabView setGLResources:nil];
    }
    
    [wadTableView reloadData];
    [self updateTextureControls];
}

- (void)updateTextureControls {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSSet* selectedFaces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    
    NSMutableSet* selectedTextureNames = nil;
    if ([selectedFaces count] > 0) {
        selectedTextureNames = [[NSMutableSet alloc] init];
        
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
        
        [selectedTextureNames addObject:textureName];
        
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
            [selectedTextureNames addObject:[face texture]];
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
    
    [textureView setSelectedTextureNames:selectedTextureNames];
    [selectedTextureNames release];
}

- (void)updateFilter {
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

@end

@implementation InspectorController

+ (InspectorController *)sharedInspector {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (void)dealloc {
    [self setMapWindowController:nil];
    [entityPropertyTableDataSource release];
    [super dealloc];
}

- (NSString *)windowNibName {
    return @"Inspector";
}

- (void)windowDidLoad {
    [super windowDidLoad];
    entityPropertyTableDataSource = [[EntityPropertyTableDataSource alloc] init];
    [entityPropertyTableView setDataSource:entityPropertyTableDataSource];
    [self updateMapWindowController:mapWindowController];
    [self prefabsPerRowChanged:prefabsPerRowSlider];
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
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces xOffset:xOffset];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Set Texture X Offset"];
}

- (IBAction)yOffsetTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];

    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    int yOffset = [yOffsetField intValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces yOffset:yOffset];

    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Set Texture Y Offset"];
}

- (IBAction)xScaleTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];

    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float xScale = [xScaleField floatValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces xScale:xScale];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Set Texture X Scale"];
}

- (IBAction)yScaleTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];

    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float yScale = [yScaleField floatValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces yScale:yScale];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Set Texture Y Scale"];
}

- (IBAction)rotationTextChanged:(id)sender {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float rotation = [rotationField floatValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces rotation:rotation];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Set Texture Rotation"];
}

- (void)textureSelected:(Texture *)texture {
    MapDocument* map = [mapWindowController document];
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    [map setFaces:faces texture:[texture name]];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Set Texture"];
}

- (IBAction)textureNameFilterTextChanged:(id)sender {
    [self updateFilter];
}

- (IBAction)textureUsageFilterChanged:(id)sender {
    [self updateFilter];
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

- (IBAction)toggleTextureControls:(id)sender {
    NSRect boxFrame = [textureControlBox frame];
    NSRect viewFrame = [textureScrollView frame];
    NSRect newBoxFrame;
    NSRect newViewFrame;
    int a = 129;
    if ([sender state] == NSOnState) {
        newBoxFrame = NSMakeRect(NSMinX(boxFrame), NSMinY(boxFrame) - a, NSWidth(boxFrame), NSHeight(boxFrame) + a);
        newViewFrame = NSMakeRect(NSMinX(viewFrame), NSMinY(viewFrame), NSWidth(viewFrame), NSHeight(viewFrame) - a);
    } else {
        newBoxFrame = NSMakeRect(NSMinX(boxFrame), NSMinY(boxFrame) + a, NSWidth(boxFrame), NSHeight(boxFrame) - a);
        newViewFrame = NSMakeRect(NSMinX(viewFrame), NSMinY(viewFrame), NSWidth(viewFrame), NSHeight(viewFrame) + a);
    }
    
    [textureControlBox setFrame:newBoxFrame];
    [textureControlBox setNeedsDisplay:YES];
    
    [textureScrollView setFrame:newViewFrame];
    [textureScrollView setNeedsDisplay:YES];
    
    [[textureControlBox superview] setNeedsDisplay:YES];
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
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSSet* entities = [selectionManager selectedEntities];
    
    MapDocument* map = [mapWindowController document];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];

    [map setEntities:entities propertyKey:@"new_property" value:@""];
    
    [undoManager setActionName:@"Add Entity Property"];
    [undoManager endUndoGrouping];
}

- (IBAction)removeEntityProperty:(id)sender {
    NSIndexSet* selectedRows = [entityPropertyTableView selectedRowIndexes];
    if ([selectedRows count] == 0)
        return;
    
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSSet* entities = [selectionManager selectedEntities];
    
    MapDocument* map = [mapWindowController document];
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

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
    NSIndexSet* selectedRows = [entityPropertyTableView selectedRowIndexes];
    if ([selectedRows count] > 0) {
        SelectionManager* selectionManager = [mapWindowController selectionManager];
        NSSet* entities = [selectionManager selectedEntities];
        
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

- (BOOL)tableView:(NSTableView *)theTableView shouldEditTableColumn:(NSTableColumn *)theTableColumn row:(NSInteger)theRowIndex {
    return [entityPropertyTableDataSource editingAllowed:theTableColumn rowIndex:theRowIndex];
}

@end

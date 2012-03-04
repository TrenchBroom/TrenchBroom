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

#import "MapWindowController.h"
#import "MapView3D.h"
#import "TextureView.h"
#import "MapDocument.h"
#import "GLResources.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "Camera.h"
#import "MapDocument.h"
#import "WadLoader.h"
#import "Wad.h"
#import "TextureManager.h"
#import "InputManager.h"
#import "Vbo.h"
#import "Octree.h"
#import "Picker.h"
#import "SelectionManager.h"
#import "GLFontManager.h"
#import "InspectorViewController.h"
#import "InspectorWindowController.h"
#import "Options.h"
#import "Grid.h"
#import "PrefabManager.h"
#import "PrefabNameSheetController.h"
#import "Prefab.h"
#import "MapWriter.h"
#import "CameraAbsoluteAnimation.h"
#import "ClipTool.h"
#import "EntityDefinitionManager.h"
#import "EntityDefinition.h"
#import "math.h"
#import "ControllerUtils.h"
#import "PreferencesController.h"
#import "PreferencesManager.h"
#import "MapCompiler.h"
#import "ConsoleWindowController.h"
#import "QuickBarWindowController.h"
#import "MapParser.h"
#import "CompilerProfileManager.h"
#import "CompilerProfile.h"
#import "PointFileFeedbackFigure.h"
#import "Renderer.h"
#import "GroupManager.h"
#import "DragPlane.h"
#import "MutableBrush.h"
#import "MutableFace.h"

@interface MapWindowController (private)

- (void)selectionRemoved:(NSNotification *)notification;
- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;
- (void)windowWillClose:(NSNotification *)notification;
- (void)preferencesDidChange:(NSNotification *)notification;
- (void)pointFileLoaded:(NSNotification *)notification;
- (void)pointFileUnloaded:(NSNotification *)notification;

- (void)runQuake:(NSString *)appPath;

@end

@implementation MapWindowController (private)

- (void)selectionRemoved:(NSNotification *)notification {
//    if ([[self selectionManager] mode] == SM_UNDEFINED)
//        [options setIsolationMode:IM_NONE];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    if ([[PreferencesManager sharedManager] inspectorSeparate]) {
        InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
        [inspector setMapWindowController:self];
    }
    [view3D becomeFirstResponder];
}

- (void)windowDidResignKey:(NSNotification *)notification {
    /*
    if ([[PreferencesManager sharedManager] inspectorSeparate]) {
        InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
        [inspector setMapWindowController:nil];
    }
     */
    [view3D resignFirstResponder];
}

- (void)windowWillClose:(NSNotification *)notification {
    if ([[PreferencesManager sharedManager] inspectorSeparate]) {
        InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
        [inspector setMapWindowController:nil];
    }
}

- (void)pointFileLoaded:(NSNotification *)notification {
    MapDocument* document = [self document];
    pointFileFigure = [[PointFileFeedbackFigure alloc] initWithPoints:[document leakPoints] pointCount:[document leakPointCount]];
    
    Renderer* renderer = [self renderer];
    [renderer addFeedbackFigure:pointFileFigure];
}

- (void)preferencesDidChange:(NSNotification *)notification {
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    
    [camera setFieldOfVision:[preferences cameraFov]];
    [camera setNearClippingPlane:[preferences cameraNear]];
    [camera setFarClippingPlane:[preferences cameraFar]];
    
    BOOL inspectorVisible = [preferences inspectorVisible];
    BOOL inspectorSeparate = [preferences inspectorSeparate];
    
    if (inspectorSeparate) {
        if (inspectorViewController != nil) {
            NSView* inspectorView = [inspectorViewController view];
            [inspectorView removeFromSuperview];
            [inspectorViewController release];
            inspectorViewController = nil;
            [preferences setInspectorVisible:YES];
            
            InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
            [inspector setMapWindowController:self];
            [[inspector window] makeKeyAndOrderFront:self];
        } else {
            InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
            [inspector setMapWindowController:self];
            
            if (inspectorVisible)
                [[inspector window] makeKeyAndOrderFront:self];
            else
                [[inspector window] orderOut:self];
        }
    } else {
        if (inspectorViewController == nil) {
            InspectorWindowController* inspector = [InspectorWindowController sharedInspector];
            [inspector setMapWindowController:nil];
            NSWindow* inspectorWindow = [inspector window];
            [inspectorWindow close];
            
            inspectorViewController = [[InspectorViewController alloc] initWithNibName:@"InspectorView" bundle:nil];
            [inspectorViewController setMapWindowController:self];
            [preferences setInspectorVisible:YES];
        }
        
        if (inspectorVisible) {
            NSView* inspectorView = [inspectorViewController view];
            [splitView addSubview:inspectorView];
            [splitView adjustSubviews];
            
            float width = [splitView frame].size.width;
            float pos = width - 402;
            [splitView setPosition:pos ofDividerAtIndex:0];
        } else {
            NSView* inspectorView = [inspectorViewController view];
            [inspectorView removeFromSuperview];
        }
    }
}

- (void)pointFileUnloaded:(NSNotification *)notification {
    Renderer* renderer = [self renderer];
    [renderer removeFeedbackFigure:pointFileFigure];
    [pointFileFigure release];
    pointFileFigure = nil;
}

- (void)runQuake:(NSString *)appPath {
    MapDocument* map = [self document];
    NSURL* mapFileUrl = [map fileURL];
    if (mapFileUrl == nil) {
        [console log:@"Map must be saved and compiled first"];
        return;
    }
    
    NSError* error;
    
    NSString* mapFilePath = [mapFileUrl path];
    NSString* mapDirPath = [mapFilePath stringByDeletingLastPathComponent];
    NSString* mapFileName = [mapFilePath lastPathComponent];
    NSString* baseFileName = [mapFileName stringByDeletingPathExtension];
    NSString* bspFileName = [baseFileName stringByAppendingPathExtension:@"bsp"];
    NSString* litFileName = [baseFileName stringByAppendingPathExtension:@"lit"];
    NSString* bspFilePath = [mapDirPath stringByAppendingPathComponent:bspFileName];
    NSString* litFilePath = [mapDirPath stringByAppendingPathComponent:litFileName];
    
    NSFileManager* fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:bspFilePath]) {
        [console logBold:[NSString stringWithFormat:@"Could not find BSP file at '%@'\n", bspFilePath]];
        return;
    }
    
    NSString* quakePath = [[PreferencesManager sharedManager] quakePath];
    if (quakePath == nil) {
        [console logBold:@"Quake path not defined"];
        return;
    }
    
    NSArray* modList = modListFromWorldspawn([map worldspawn:YES]);
    NSString* modPath = [quakePath stringByAppendingPathComponent:[modList lastObject]];
    
    BOOL directory;
    BOOL exists = [fileManager fileExistsAtPath:modPath isDirectory:&directory];
    if (!exists || !directory) {
        [console logBold:[NSString stringWithFormat:@"Mod path '%@' does not exist or is not a directory\n", modPath]];
        return;
    }
    
    NSString* modMapsDirPath = [modPath stringByAppendingPathComponent:@"maps"];
    exists = [fileManager fileExistsAtPath:modMapsDirPath isDirectory:&directory];
    if (exists && !directory) {
        [console logBold:[NSString stringWithFormat:@"Mod maps path '%@' is not a directory\n", modMapsDirPath]];
        return;
    }
    
    if (!exists) {
        [console log:[NSString stringWithFormat:@"creating mod maps directory at '%@'\n", modMapsDirPath]];
        if (![fileManager createDirectoryAtPath:modMapsDirPath withIntermediateDirectories:NO attributes:nil error:&error]) {
            [console logBold:[NSString stringWithFormat:@"Failed to create mod maps directory at '%@': %@\n", modMapsDirPath, [error localizedDescription]]];
            return;
        }
    }
    
    NSString* targetBspPath = [modMapsDirPath stringByAppendingPathComponent:bspFileName];
    if ([fileManager fileExistsAtPath:targetBspPath]) {
        [console log:[NSString stringWithFormat:@"Removing existing BSP file '%@'\n", targetBspPath]];
        if (![fileManager removeItemAtPath:targetBspPath error:&error]) {
            [console logBold:[NSString stringWithFormat:@"Failed to remove existing BSP file from '%@': %@\n", targetBspPath, [error localizedDescription]]];
            return;
        }
    }
    
    if (![fileManager copyItemAtPath:bspFilePath toPath:targetBspPath error:&error]) {
        [console logBold:[NSString stringWithFormat:@"Failed to copy BSP file from '%@' to '%@': %@\n", bspFilePath, targetBspPath, [error localizedDescription]]];
        return;
    }

    NSString* targetLitPath = [modMapsDirPath stringByAppendingPathComponent:litFileName];
    if ([fileManager fileExistsAtPath:targetLitPath]) {
        [console log:[NSString stringWithFormat:@"Removing existing lightmap file '%@'\n", targetLitPath]];
        if (![fileManager removeItemAtPath:targetLitPath error:&error]) {
            [console logBold:[NSString stringWithFormat:@"Failed to remove existing lightmap file from '%@': %@\n", targetLitPath, [error localizedDescription]]];
            return;
        }
    }
    
    if ([fileManager fileExistsAtPath:litFilePath]) {
        if (![fileManager copyItemAtPath:litFilePath toPath:targetLitPath error:&error]) {
            [console logBold:[NSString stringWithFormat:@"Failed to copy lightmap file from '%@' to '%@': %@\n", litFilePath, targetLitPath, [error localizedDescription]]];
            return;
        }
    }
    
    NSURL* appUrl = [NSURL fileURLWithPath:appPath];
    NSDictionary *config = [NSDictionary dictionaryWithObjectsAndKeys:[NSArray arrayWithObjects:@"-nolauncher", @"+map", baseFileName, nil], NSWorkspaceLaunchConfigurationArguments, nil];

    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    if ([workspace launchApplicationAtURL:appUrl options:NSWorkspaceLaunchNewInstance | NSWorkspaceLaunchDefault configuration:config error:&error] != nil)
        [[PreferencesManager sharedManager] setLastExecutablePath:appPath];
    else
        [console logBold:[NSString stringWithFormat:@"Failed to launch executable '%@‘: %@\n", appPath, [error localizedDescription]]];
}

@end

@implementation MapWindowController

- (void)windowWillBeginSheet:(NSNotification *)notification {
    view3DWasFirstResponder = [[self window] firstResponder] == view3D;
    if (view3DWasFirstResponder)
        [view3D resignFirstResponder];
}

- (void)windowDidEndSheet:(NSNotification *)notification {
    if (view3DWasFirstResponder)
        [view3D becomeFirstResponder];
    view3DWasFirstResponder = NO;
}

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self document] undoManager];
}

- (void)setDocument:(NSDocument *)document {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    
    NSDocument* oldDocument = [self document];
    if (oldDocument != nil) {
        [center removeObserver:self name:PointFileLoaded object:oldDocument];
        [center removeObserver:self name:PointFileUnloaded object:oldDocument];
    }
    
    if (document != nil) {
        [center addObserver:self selector:@selector(pointFileLoaded:) name:PointFileLoaded object:document];
        [center addObserver:self selector:@selector(pointFileUnloaded:) name:PointFileUnloaded object:document];
    }
    
    [super setDocument:document];
}

- (CGFloat)splitView:(NSSplitView *)theSplitView constrainMaxCoordinate:(CGFloat)theProposedMax ofSubviewAt:(NSInteger)theDividerIndex {
    return NSWidth([splitView frame]) - 402;
}

- (void)windowDidResize:(NSNotification *)notification {
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    if (![preferences inspectorSeparate] && NSWidth([[inspectorViewController view] frame]) < 402)
        [splitView setPosition:NSWidth([splitView frame]) - 402 ofDividerAtIndex:0];
}

- (void)windowDidLoad {
    console = [[ConsoleWindowController alloc] initWithWindowNibName:@"ConsoleWindow"];
    
    GLResources* glResources = [[self document] glResources];
    NSOpenGLContext* sharedContext = [glResources openGLContext];
    NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[view3D pixelFormat] shareContext:sharedContext];
    [view3D setOpenGLContext:sharingContext];
    [sharingContext release];
    
    options = [[Options alloc] init];
    camera = [[Camera alloc] init];
    [self preferencesDidChange:nil];
    
    inputManager = [[InputManager alloc] initWithWindowController:self];
    
    [view3D setup];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:[self window]];
    [center addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:[self window]];
    [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:[PreferencesManager sharedManager]];
    [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:[self selectionManager]];
    
    [[self window] setDelegate:self];
    [[self window] setAcceptsMouseMovedEvents:YES];
    [[self window] makeKeyAndOrderFront:self];

    quickBar = [[QuickBarWindowController alloc] initWithWindowNibName:@"QuickBarWindow"];
    [quickBar setMapWindowController:self];
    [[self window] addChildWindow:[quickBar window] ordered:NSWindowAbove];
    [[quickBar window] orderFront:self];
    
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    SEL action = [menuItem action];
    SelectionManager* selectionManager = [self selectionManager];
    
    if (action == @selector(selectAll:)) {
        return YES;
    } else if (action == @selector(selectNone:)) {
        return [selectionManager hasSelection];
    } else if (action == @selector(selectEntity:)) {
        id <Entity> entity = [selectionManager brushSelectionEntity];
        return ![entity isWorldspawn];
    } else if (action == @selector(selectAllTouchingBrush:)) {
        return [selectionManager mode] == SM_BRUSHES && [[selectionManager selectedBrushes] count] == 1; 
    } else if (action == @selector(copy:)) {
        return [selectionManager hasSelection];
    } else if (action == @selector(cut:)) {
        return [selectionManager hasSelection];
    } else if (action == @selector(paste:)) {
        return YES;
    } else if (action == @selector(deleteSelection:)) {
        if ([selectionManager hasSelectedEntities])
            return YES;
        if ([selectionManager hasSelectedBrushes])
            return YES;
        if ([selectionManager hasSelectedFaces]) {
            for (MutableFace* face in [selectionManager selectedFaces]) {
                MutableBrush* brush = (MutableBrush *)[face brush];
                if (![brush canDeleteFace:face])
                    return NO;
            }
            
            return YES;
        }
        
        if ([[inputManager clipTool] active] && [[inputManager clipTool] numPoints] > 0)
            return YES;
        return NO;
    } else if (action == @selector(moveTextureLeft:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(moveTextureLeft:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(moveTextureRight:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(moveTextureUp:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(moveTextureDown:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(stretchTextureHorizontally:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(shrinkTextureHorizontally:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(stretchTextureVertically:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(shrinkTextureVertically:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(rotateTextureLeft:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(rotateTextureRight:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedFaces];
    } else if (action == @selector(toggleTextureLock:)) {
        return YES;
    } else if (action == @selector(duplicateSelection:)) {
        return ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]) && ![[inputManager clipTool] active];
    } else if (action == @selector(createPrefabFromSelection:)) {
        return [selectionManager hasSelectedBrushes];
    } else if (action == @selector(showInspector:)) {
        return YES;
    } else if (action == @selector(switchToXYView:)) {
        return YES;
    } else if (action == @selector(switchToXZView:)) {
        return YES;
    } else if (action == @selector(switchToYZView:)) {
        return YES;
    } else if (action == @selector(isolateSelection:)) {
        return YES;
    } else if (action == @selector(toggleProjection:)) {
        return YES;
    } else if (action == @selector(toggleGrid:)) {
        return YES;
    } else if (action == @selector(toggleSnap:)) {
        return YES;
    } else if (action == @selector(setGridSize:)) {
        return YES;
    } else if (action == @selector(toggleClipTool:)) {
        return [selectionManager hasSelectedBrushes] || [[inputManager clipTool] active];
    } else if (action == @selector(toggleClipMode:)) {
        return [[inputManager clipTool] active];
    } else if (action == @selector(performClip:)) {
        return [[inputManager clipTool] active] && [[inputManager clipTool] numPoints] > 1;
    } else if (action == @selector(rotate90CW:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities];
    } else if (action == @selector(rotate90CCW:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities];
    } else if (action == @selector(flipHorizontally:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities];
    } else if (action == @selector(flipVertically:)) {
        return [selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities];
    } else if (action == @selector(createPointEntity:)) {
        return YES;
    } else if (action == @selector(createBrushEntity:)) {
        id <Entity> entity = [selectionManager brushSelectionEntity];
        return [entity isWorldspawn];
    } else if (action == @selector(compile:)) {
        return YES;
    } else if (action == @selector(run:)) {
        return YES;
    } else if (action == @selector(compileMostRecent:)) {
        int index = [[PreferencesManager sharedManager] lastCompilerProfileIndex];
        CompilerProfileManager* profileManager = [CompilerProfileManager sharedManager];
        return index >= 0 && index < [[profileManager profiles] count];
    } else if (action == @selector(runDefaultEngine:)) {
        return [[PreferencesManager sharedManager] quakeExecutable] != nil;
    } else if (action == @selector(loadPointFile:)) {
        MapDocument* map = [self document];
        NSURL* mapFileUrl = [map fileURL];
        
        NSString* mapFilePath = [mapFileUrl path];
        NSString* mapDirPath = [mapFilePath stringByDeletingLastPathComponent];
        NSString* mapFileName = [mapFilePath lastPathComponent];
        NSString* baseFileName = [mapFileName stringByDeletingPathExtension];
        NSString* pointFileName = [baseFileName stringByAppendingPathExtension:@"pts"];
        NSString* pointFilePath = [mapDirPath stringByAppendingPathComponent:pointFileName];
        
        NSFileManager* fileManager = [NSFileManager defaultManager];
        return [fileManager fileExistsAtPath:pointFilePath];
    } else if (action == @selector(unloadPointFile:)) {
        MapDocument* map = [self document];
        return [map leakPointCount] > 0;
    } else if (action == @selector(enlargeBrushes:)) {
        return [selectionManager hasSelectedBrushes];
    } else if (action == @selector(toggleDragVertexTool:)) {
        return YES;
    } else if (action == @selector(toggleDragEdgeTool:)) {
        return YES;
    } else if (action == @selector(toggleDragFaceTool:)) {
        return YES;
    }

    return NO;
}

- (void)prefabNameSheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo {
    PrefabNameSheetController* pns = [sheet windowController];
    if (returnCode == NSOKButton) {
        NSString* prefabName = [pns prefabName];
        NSString* prefabGroupName = [pns prefabGroup];
        
        SelectionManager* selectionManager = [self selectionManager];
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
        id <PrefabGroup> prefabGroup = [prefabManager prefabGroupWithName:prefabGroupName create:YES];
        [prefabManager createPrefabFromBrushTemplates:[selectionManager selectedBrushes] name:prefabName group:prefabGroup];
    }
    [pns release];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [pointFileFigure release];
    [options release];
    [inputManager release];
    [camera release];
    [quickBar release];
    [inspectorViewController release];
    [console release];
    [super dealloc];
}

#pragma mark Point File Support

- (IBAction)loadPointFile:(id)sender {
    MapDocument* map = [self document];
    NSURL* mapFileUrl = [map fileURL];
    
    NSString* mapFilePath = [mapFileUrl path];
    NSString* mapDirPath = [mapFilePath stringByDeletingLastPathComponent];
    NSString* mapFileName = [mapFilePath lastPathComponent];
    NSString* baseFileName = [mapFileName stringByDeletingPathExtension];
    NSString* pointFileName = [baseFileName stringByAppendingPathExtension:@"pts"];
    NSString* pointFilePath = [mapDirPath stringByAppendingPathComponent:pointFileName];
    
    NSLog(@"Loading point file %@", pointFilePath);
    
    NSError* error;
    NSData* data = [NSData dataWithContentsOfFile:pointFilePath options:NSDataReadingMapped error:&error];
    if (data == nil) {
        NSLog(@"Could not read point file: %@", [error localizedDescription]);
        return;
    }
    
    [map loadPointFile:data];
    NSLog(@"Point file loaded");
}

- (IBAction)unloadPointFile:(id)sender {
    NSLog(@"Loading point file");
    [[self document] unloadPointFile];
}

#pragma mark Entity related actions

- (IBAction)createPointEntity:(id)sender {
    MapDocument* map = [self document];
    EntityDefinitionManager* entityDefinitionManager = [map entityDefinitionManager];
    SelectionManager* selectionManager = [self selectionManager];
    
    NSArray* pointDefinitions = [entityDefinitionManager definitionsOfType:EDT_POINT];
    EntityDefinition* definition = [pointDefinitions objectAtIndex:[sender tag]];

    NSPoint mousePos = [inputManager menuPosition];
    PickingHitList* hits = [inputManager currentHits];
    
    Grid* grid = [options grid];

    TVector3i insertPoint;
    calculateEntityOrigin(grid, definition, hits, mousePos, camera, &insertPoint);
    
    NSString* origin = [NSString stringWithFormat:@"%i %i %i", insertPoint.x, insertPoint.y, insertPoint.z];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    [selectionManager removeAll:YES];
    
    id <Entity> entity = [map createEntityWithClassname:[definition name]];
    [map setEntity:entity propertyKey:OriginKey value:origin];
    
    [selectionManager addEntity:entity record:YES];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Create Point Entity"];
}

- (IBAction)createBrushEntity:(id)sender {
    MapDocument* map = [self document];
    EntityDefinitionManager* entityDefinitionManager = [map entityDefinitionManager];
    SelectionManager* selectionManager = [self selectionManager];
    
    NSArray* brushes = [selectionManager selectedBrushes];
    if ([brushes count] == 0)
        return;
    
    NSArray* brushDefinitions = [entityDefinitionManager definitionsOfType:EDT_BRUSH];
    EntityDefinition* definition = [brushDefinitions objectAtIndex:[sender tag]];
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    id <Entity> entity = [map createEntityWithClassname:[definition name]];
    [map moveBrushesToEntity:entity brushes:brushes];
    [selectionManager addEntity:entity record:YES];

    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Create Brush Entity"];
}


#pragma mark Brush related actions

- (IBAction)rotate90CW:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    
    EAxis axis;
    float comp;
    TVector3f centerf;
    TBoundingBox bounds;

    axis = strongestComponentV3f([camera direction]);
    comp = componentV3f([camera direction], axis);
    [selectionManager selectionBounds:&bounds];
    centerOfBounds(&bounds, &centerf);
    
    if (comp < 0)
        rotateBounds90CW(&bounds, axis, &centerf, &bounds);
    else
        rotateBounds90CCW(&bounds, axis, &centerf, &bounds);
    
    if (boundsContainBounds([map worldBounds], &bounds)) {
        roundV3f(&centerf, &centerf);
        
        NSUndoManager* undoManager = [map undoManager];
        [undoManager beginUndoGrouping];
        
        if (comp < 0) {
            [map rotateBrushes90CW:[selectionManager selectedBrushes] axis:axis center:centerf lockTextures:[options lockTextures]];
            [map rotateEntities90CW:[selectionManager selectedEntities] axis:axis center:centerf];
        } else {
            [map rotateBrushes90CCW:[selectionManager selectedBrushes] axis:axis center:centerf lockTextures:[options lockTextures]];
            [map rotateEntities90CCW:[selectionManager selectedEntities] axis:axis center:centerf];
        }
        
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Rotate Objects 90° Clockwise"];
    }
}

- (IBAction)rotate90CCW:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    
    EAxis axis;
    float comp;
    TVector3f center;
    TBoundingBox bounds;
    
    axis = strongestComponentV3f([camera direction]);
    comp = componentV3f([camera direction], axis);
    [selectionManager selectionBounds:&bounds];
    centerOfBounds(&bounds, &center);

    if (comp > 0)
        rotateBounds90CW(&bounds, axis, &center, &bounds);
    else
        rotateBounds90CCW(&bounds, axis, &center, &bounds);

    if (boundsContainBounds([map worldBounds], &bounds)) {
        NSUndoManager* undoManager = [map undoManager];
        [undoManager beginUndoGrouping];
        
        if (comp > 0) {
            [map rotateBrushes90CW:[selectionManager selectedBrushes] axis:axis center:center lockTextures:[options lockTextures]];
            [map rotateEntities90CW:[selectionManager selectedEntities] axis:axis center:center];
        } else {
            [map rotateBrushes90CCW:[selectionManager selectedBrushes] axis:axis center:center lockTextures:[options lockTextures]];
            [map rotateEntities90CCW:[selectionManager selectedEntities] axis:axis center:center];
        }
        
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Rotate Objects 90° Counterclockwise"];
    }
}

- (IBAction)flipHorizontally:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    
    EAxis axis = strongestComponentV3f([camera right]);
    
    TVector3f center;
    TBoundingBox bounds;

    [selectionManager selectionBounds:&bounds];
    centerOfBounds(&bounds, &center);
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    [map flipBrushes:[selectionManager selectedBrushes] axis:axis center:center lockTextures:[options lockTextures]];
    [map flipEntities:[selectionManager selectedEntities] axis:axis center:center];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:[NSString stringWithFormat:@"flip Objects Along %@ Axis", axisName(axis)]];
}

- (IBAction)flipVertically:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    
    EAxis axis = strongestComponentV3f([camera up]);
    
    TVector3f center;
    TBoundingBox bounds;
    
    [selectionManager selectionBounds:&bounds];
    centerOfBounds(&bounds, &center);
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    [map flipBrushes:[selectionManager selectedBrushes] axis:axis center:center lockTextures:[options lockTextures]];
    [map flipEntities:[selectionManager selectedEntities] axis:axis center:center];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:[NSString stringWithFormat:@"flip Objects Along %@ Axis", axisName(axis)]];
}

- (IBAction)toggleDragVertexTool:(id)sender {
    [inputManager toggleDragVertexTool];
}

- (IBAction)toggleDragEdgeTool:(id)sender {
    [inputManager toggleDragEdgeTool];
}

- (IBAction)toggleDragFaceTool:(id)sender {
    [inputManager toggleDragFaceTool];
}

- (IBAction)toggleClipTool:(id)sender {
    ClipTool* clipTool = [inputManager clipTool];
    if ([clipTool active])
        [clipTool deactivate];
    else
        [clipTool activate];
}

- (IBAction)toggleClipMode:(id)sender {
    ClipTool* clipTool = [inputManager clipTool];
    if ([clipTool active])
        [clipTool toggleClipMode];
}

- (IBAction)performClip:(id)sender {
    ClipTool* clipTool = [inputManager clipTool];
    if ([clipTool active]) {
        [clipTool performClip:[self document]];
        [clipTool deactivate];
    }
}

- (IBAction)enlargeBrushes:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    Grid* grid = [options grid];
    
    [map enlargeBrushes:[selectionManager selectedBrushes] by:[grid actualSize] lockTextures:[options lockTextures]];
}

#pragma mark Face related actions

- (IBAction)toggleTextureLock:(id)sender {
    [options setLockTextures:![options lockTextures]];
}

#pragma mark Shared actions

- (void)moveLeft:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float dist = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces]) {
        if (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
            dist = 1;

        TVector3f dir;
        invertV3f([camera right], &dir);
        [map translateFaceOffsets:[selectionManager selectedFaces] delta:dist dir:dir];
    }
    
    if ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]) {
        TVector3f delta;
        DragPlane* editingSystem = [camera horizontalEditingSystem];
        scaleV3f([editingSystem xAxisPos], -dist, &delta);

        const TBoundingBox* worldBounds = [map worldBounds];
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        
        [[options grid] moveDeltaForBounds:&bounds worldBounds:worldBounds delta:&delta lastPoint:NULL];

        if (nullV3f(&delta))
            return;
        
        [map translateBrushes:[selectionManager selectedBrushes] delta:delta lockTextures:[options lockTextures]];
        [map translateEntities:[selectionManager selectedEntities] delta:delta];
    }
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];
}

- (void)moveRight:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float dist = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces]) {
        if (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
            dist = 1;
        
        TVector3f dir = *[camera right];
        [map translateFaceOffsets:[selectionManager selectedFaces] delta:dist dir:dir];
    }
    
    
    if ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]) {
        TVector3f delta;
        DragPlane* editingSystem = [camera horizontalEditingSystem];
        scaleV3f([editingSystem xAxisPos], dist, &delta);
        
        const TBoundingBox* worldBounds = [map worldBounds];
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        
        [[options grid] moveDeltaForBounds:&bounds worldBounds:worldBounds delta:&delta lastPoint:NULL];
        
        if (nullV3f(&delta))
            return;
        
        [map translateBrushes:[selectionManager selectedBrushes] delta:delta lockTextures:[options lockTextures]];
        [map translateEntities:[selectionManager selectedEntities] delta:delta];
    }
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];}

- (void)moveUp:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float dist = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces]) {
        if (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
            dist = 1;
        
        TVector3f dir = *[camera up];
        [map translateFaceOffsets:[selectionManager selectedFaces] delta:dist dir:dir];
    }
    
    if ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]) {
        TVector3f delta;
        DragPlane* editingSystem = [camera horizontalEditingSystem];
        scaleV3f([editingSystem yAxisPos], dist, &delta);
        
        const TBoundingBox* worldBounds = [map worldBounds];
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        
        [[options grid] moveDeltaForBounds:&bounds worldBounds:worldBounds delta:&delta lastPoint:NULL];
        
        if (nullV3f(&delta))
            return;
        
        [map translateBrushes:[selectionManager selectedBrushes] delta:delta lockTextures:[options lockTextures]];
        [map translateEntities:[selectionManager selectedEntities] delta:delta];
    }
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];
}

- (void)moveDown:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float dist = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces]) {
        if (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
            dist = 1;
        
        TVector3f dir;
        invertV3f([camera up], &dir);
        [map translateFaceOffsets:[selectionManager selectedFaces] delta:dist dir:dir];
    }
    
    if ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]) {
        TVector3f delta;
        DragPlane* editingSystem = [camera horizontalEditingSystem];
        scaleV3f([editingSystem yAxisPos], -dist, &delta);
        
        const TBoundingBox* worldBounds = [map worldBounds];
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        
        [[options grid] moveDeltaForBounds:&bounds worldBounds:worldBounds delta:&delta lastPoint:NULL];
        
        if (nullV3f(&delta))
            return;
        
        [map translateBrushes:[selectionManager selectedBrushes] delta:delta lockTextures:[options lockTextures]];
        [map translateEntities:[selectionManager selectedEntities] delta:delta];
    }
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];
}

- (void)pageUp:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float dist = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces]) {
        float angle = -15;
        if (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
            angle = -1;
        
        [map rotateFaces:[selectionManager selectedFaces] angle:angle];
    }
    
    if ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]) {
        TVector3f delta;
        DragPlane* editingSystem = [camera horizontalEditingSystem];
        scaleV3f([editingSystem zAxisPos], dist, &delta);
        
        const TBoundingBox* worldBounds = [map worldBounds];
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        
        [[options grid] moveDeltaForBounds:&bounds worldBounds:worldBounds delta:&delta lastPoint:NULL];
        
        if (nullV3f(&delta))
            return;
        
        [map translateBrushes:[selectionManager selectedBrushes] delta:delta lockTextures:[options lockTextures]];
        [map translateEntities:[selectionManager selectedEntities] delta:delta];
    }
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];
}

- (void)pageDown:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    float dist = [[options grid] actualSize];
    
    if ([selectionManager hasSelectedFaces]) {
        float angle = 15;
        if (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
            angle = 1;
        
        [map rotateFaces:[selectionManager selectedFaces] angle:angle];
    }

    if ([selectionManager hasSelectedBrushes] || [selectionManager hasSelectedEntities]) {
        TVector3f delta;
        DragPlane* editingSystem = [camera horizontalEditingSystem];
        scaleV3f([editingSystem zAxisPos], -dist, &delta);
        
        const TBoundingBox* worldBounds = [map worldBounds];
        TBoundingBox bounds;
        [selectionManager selectionBounds:&bounds];
        
        [[options grid] moveDeltaForBounds:&bounds worldBounds:worldBounds delta:&delta lastPoint:NULL];
        
        if (nullV3f(&delta))
            return;
        
        [map translateBrushes:[selectionManager selectedBrushes] delta:delta lockTextures:[options lockTextures]];
        [map translateEntities:[selectionManager selectedEntities] delta:delta];
    }
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Objects"];
}

#pragma mark View related actions

- (IBAction)showInspector:(id)sender {
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    [preferences setInspectorVisible:![preferences inspectorVisible]];
}

- (IBAction)toggleGrid:(id)sender {
    [[options grid] toggleDraw];
}

- (IBAction)toggleSnap:(id)sender {
    [[options grid] toggleSnap];
}

- (IBAction)setGridSize:(id)sender {
    [[options grid] setSize:[sender tag]];
}

- (IBAction)isolateSelection:(id)sender {
    EIsolationMode isolationMode = [options isolationMode];
    [options setIsolationMode:(isolationMode + 1) % 3];
}

- (IBAction)toggleProjection:(id)sender {
    ECameraMode cameraMode = [camera mode];
    [camera setMode:(cameraMode + 1) % 2];
}

- (IBAction)switchToXYView:(id)sender {
    SelectionManager* selectionManager = [self selectionManager];
    TVector3f center, diff, position;
    
    if (![selectionManager selectionCenter:&center]) {
        center = *[camera direction];
        scaleV3f(&center, 256, &center);
        addV3f(&center, [camera position], &center);
    }
    
    subV3f(&center, [camera position], &diff);
    position = center;
    position.z += lengthV3f(&diff);
    
    CameraAbsoluteAnimation* animation = [[CameraAbsoluteAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

- (IBAction)switchToXZView:(id)sender {
    SelectionManager* selectionManager = [self selectionManager];
    TVector3f center, diff, position;
    
    if (![selectionManager selectionCenter:&center]) {
        center = *[camera direction];
        scaleV3f(&center, 256, &center);
        addV3f(&center, [camera position], &center);
    }
    
    subV3f(&center, [camera position], &diff);
    position = center;
    position.y -= lengthV3f(&diff);

    CameraAbsoluteAnimation* animation = [[CameraAbsoluteAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

- (IBAction)switchToYZView:(id)sender {
    SelectionManager* selectionManager = [self selectionManager];
    TVector3f center, diff, position;
    
    if (![selectionManager selectionCenter:&center]) {
        center = *[camera direction];
        scaleV3f(&center, 256, &center);
        addV3f(&center, [camera position], &center);
    }
    
    subV3f(&center, [camera position], &diff);
    position = center;
    position.x += lengthV3f(&diff);
    
    CameraAbsoluteAnimation* animation = [[CameraAbsoluteAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

#pragma mark Structure and selection

- (IBAction)selectAll:(id)sender {
    SelectionManager* selectionManager = [self selectionManager];
    [selectionManager removeAll:NO];
    
    NSMutableArray* entities = [[NSMutableArray alloc] init];
    NSMutableArray* brushes = [[NSMutableArray alloc] init];

    MapDocument* map = [self document];
    for (id <Entity> entity in [map entities]) {
        if ([entity entityDefinition] != nil && [[entity entityDefinition] type] == EDT_POINT)
            [entities addObject:entity];
        else
            [brushes addObjectsFromArray:[entity brushes]];
    }
    
    [selectionManager addEntities:entities record:NO];
    [selectionManager addBrushes:brushes record:NO];
    
    [entities release];
    [brushes release];
}

- (IBAction)selectNone:(id)sender {
    SelectionManager* selectionManager = [self selectionManager];
    [selectionManager removeAll:NO];
}

- (IBAction)selectEntity:(id)sender {
    SelectionManager* selectionManager = [self selectionManager];
    id <Brush> brush = [[[selectionManager selectedBrushes] objectEnumerator] nextObject];
    id <Entity> entity = [brush entity];
    
    [selectionManager removeAll:NO];
    [selectionManager addEntity:entity record:NO];
    [selectionManager addBrushes:[entity brushes] record:NO];
}

- (IBAction)selectAllTouchingBrush:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];
    id <Brush> selectionBrush = [[[selectionManager selectedBrushes] objectEnumerator] nextObject];

    NSMutableArray* touchingEntities = [[NSMutableArray alloc] init];
    NSMutableArray* touchingBrushes = [[NSMutableArray alloc] init];
    
    for (id <Entity> entity in [map entities]) {
        if (![entity isWorldspawn] && [selectionBrush intersectsEntity:entity])
            [touchingEntities addObject:entity];
        
        for (id <Brush> brush in [entity brushes])
            if (selectionBrush != brush && [selectionBrush intersectsBrush:brush])
                [touchingBrushes addObject:brush];
    }
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];
    
    [map deleteBrushes:[NSSet setWithObject:selectionBrush]];
    
    if ([touchingEntities count] > 0)
        [selectionManager addEntities:touchingEntities record:YES];
    if ([touchingBrushes count] > 0)
        [selectionManager addBrushes:touchingBrushes record:YES];
    
    [undoManager setActionName:@"Select Touching"];
    [undoManager endUndoGrouping];
}

- (void)copy:(id)sender {
    SelectionManager* selectionManager = [[self document] selectionManager];
    NSOutputStream* stream = [NSOutputStream outputStreamToMemory];

    MapWriter* mapWriter = [[MapWriter alloc] initWithSelection:selectionManager];
    [stream open];
    [mapWriter writeToStream:stream];
    [stream close];
    [mapWriter release];
    
    NSData* streamData = [stream propertyForKey:NSStreamDataWrittenToMemoryStreamKey];
    NSString* string = [[NSString alloc] initWithData:streamData encoding:NSASCIIStringEncoding];
    
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
    
    NSArray* pasteboardObjects = [[NSArray alloc] initWithObjects:string, nil];
    [pasteboard writeObjects:pasteboardObjects];
    [pasteboardObjects release];
}

- (void)cut:(id)sender {
    [self copy:sender];
    [self deleteSelection:sender];
}

- (void)paste:(id)sender {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    NSArray* objects = [pasteboard readObjectsForClasses:[NSArray arrayWithObject:[NSString class]] options:nil];
    NSString* string = [objects objectAtIndex:0];
    NSData* stringData = [string dataUsingEncoding:NSASCIIStringEncoding];
    
    MapParser* mapParser = [[MapParser alloc] initWithData:stringData];
    NSMutableArray* mapObjects = [[NSMutableArray alloc] init];
    
    MapDocument* map = [self document];
    const TBoundingBox* worldBounds = [map worldBounds];
    
    GLResources* glResources = [map glResources];
    TextureManager* textureManager = [glResources textureManager];
    
    EClipboardContents contents = [mapParser parseClipboard:mapObjects worldBounds:worldBounds textureManager:textureManager];
    [mapParser release];
    
    if (contents == CC_UNDEFINED)
        return;
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];

    SelectionManager* selectionManager = [map selectionManager];
    
    NSMutableArray* newEntities = [[NSMutableArray alloc] init];
    NSMutableArray* newBrushes = [[NSMutableArray alloc] init];
    
    switch (contents) {
        case CC_ENT: {
            [selectionManager removeAll:YES];
            for (id <Entity> entity in mapObjects) {
                id <Entity> targetEntity;
                if ([entity isWorldspawn]) {
                    targetEntity = [map worldspawn:YES];
                } else {
                    targetEntity = [map createEntityWithProperties:[entity properties]];
                    [newEntities addObject:targetEntity];
                }
                
                for (id <Brush> brush in [entity brushes]) {
                    id <Brush> newBrush = [map createBrushInEntity:targetEntity fromTemplate:brush];
                    [newBrushes addObject:newBrush];
                }
            }
            break;
        }
        case CC_BRUSH: {
            [selectionManager removeAll:YES];
            id <Entity> worldspawn = [map worldspawn:YES];
            for (id <Brush> brush in mapObjects) {
                id <Brush> newBrush = [map createBrushInEntity:worldspawn fromTemplate:brush];
                [newBrushes addObject:newBrush];
            }
            break;
        }
        case CC_FACE: {
            NSArray* faces;
            if ([selectionManager mode] == SM_BRUSHES)
                faces = [selectionManager selectedBrushFaces];
            else if ([selectionManager mode] == SM_FACES)
                faces = [selectionManager selectedFaces];
            else
                return;
            
            id <Face> source = [mapObjects lastObject];
            [map setFaces:faces texture:[source texture]];
            if (([NSEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask) {
                [map setFaces:faces xOffset:[source xOffset]];
                [map setFaces:faces yOffset:[source yOffset]];
                [map setFaces:faces xScale:[source xScale]];
                [map setFaces:faces yScale:[source yScale]];
                [map setFaces:faces rotation:[source rotation]];
            }
            break;
        }
            
        default:
            break;
    }
    
    if (contents != CC_FACE) {
        [selectionManager addEntities:newEntities record:YES];
        [selectionManager addBrushes:newBrushes record:YES];
        
        if (([NSEvent modifierFlags] & NSAlternateKeyMask) == 0) {
            TBoundingBox bounds;
            [selectionManager selectionBounds:&bounds];
            
            TVector3f oldCenter;
            centerOfBounds(&bounds, &oldCenter);
            
            TVector3f newCenter = [camera defaultPoint];
            [[options grid] snapToGridV3f:&newCenter result:&newCenter];
            
            TVector3f delta;
            subV3f(&newCenter, &oldCenter, &delta);
            
            [map translateEntities:newEntities delta:delta];
            [map translateBrushes:newBrushes delta:delta lockTextures:[options lockTextures]];
        }
    }
    
    [undoManager setActionName:@"Paste Objects"];
    [undoManager endUndoGrouping];
    [mapObjects release];
}

- (IBAction)deleteSelection:(id)sender {
    SelectionManager* selectionManager = [self selectionManager];
    ClipTool* clipTool = [inputManager clipTool];
    if ([clipTool active] && [clipTool numPoints] > 0) {
        [clipTool deleteLastPoint];
    } else {
        MapDocument* map = [self document];
        
        NSArray* deletedEntities = [[NSArray alloc] initWithArray:[selectionManager selectedEntities]];
        NSMutableArray* deletedBrushes = [[NSMutableArray alloc] initWithArray:[selectionManager selectedBrushes]];
        NSMutableArray* remainingBrushes = [[NSMutableArray alloc] init];
        
        for (id <Brush> brush in deletedBrushes) {
            id <Entity> entity = [brush entity];
            if ([deletedEntities indexOfObjectIdenticalTo:entity] != NSNotFound)
                [remainingBrushes addObject:brush];
        }
        
        [deletedBrushes removeObjectsInArray:remainingBrushes];
        
        NSUndoManager* undoManager = [[self document] undoManager];
        [undoManager beginUndoGrouping];
        
        if ([remainingBrushes count] > 0)
            [[self document] moveBrushesToEntity:[map worldspawn:YES] brushes:remainingBrushes];
        
        [[self document] deleteEntities:deletedEntities];
        [[self document] deleteBrushes:deletedBrushes];

        [selectionManager addBrushes:remainingBrushes record:YES];

        [deletedEntities release];
        [deletedBrushes release];
        [remainingBrushes release];
        
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Delete Selection"];
    }
}

- (IBAction)duplicateSelection:(id)sender {
    MapDocument* map = [self document];
    SelectionManager* selectionManager = [self selectionManager];

    NSMutableArray* newEntities = [[NSMutableArray alloc] init];
    NSMutableArray* newBrushes = [[NSMutableArray alloc] init];

    Grid* grid = [options grid];
    
    TVector3f delta = NullVector;
    const TVector3f* cameraDirection = [camera direction];
    EAxis strongestComponent = strongestComponentV3f(cameraDirection);
    setComponentV3f(&delta, strongestComponent, componentV3f(cameraDirection, strongestComponent));
    normalizeV3f(&delta, &delta);
    scaleV3f(&delta, -[grid actualSize], &delta);
    [grid snapToFarthestGridV3f:&delta result:&delta];

    const TBoundingBox* worldBounds = [map worldBounds];
    TBoundingBox bounds;
    [selectionManager selectionBounds:&bounds];
    if (bounds.max.x + delta.x > worldBounds->max.x || bounds.min.x + delta.x < worldBounds->min.x)
        delta.x *= -1;
    if (bounds.max.y + delta.y > worldBounds->max.y || bounds.min.y + delta.y < worldBounds->min.y)
        delta.y *= -1;
    if (bounds.max.z + delta.z > worldBounds->max.z || bounds.min.z + delta.z < worldBounds->min.z)
        delta.z *= -1;
    
    NSUndoManager* undoManager = [map undoManager];
    [undoManager beginUndoGrouping];

    [map duplicateEntities:[selectionManager selectedEntities] newEntities:newEntities newBrushes:newBrushes];
    [map duplicateBrushes:[selectionManager selectedBrushes] newBrushes:newBrushes];
    
    [selectionManager removeAll:YES];
    
    [selectionManager addEntities:newEntities record:YES];
    [selectionManager addBrushes:newBrushes record:YES];

    [map translateEntities:newEntities delta:delta];
    [map translateBrushes:newBrushes delta:delta lockTextures:[options lockTextures]];
    
    [newEntities release];
    [newBrushes release];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Duplicate Selection"];
}


- (IBAction)createPrefabFromSelection:(id)sender {
    PrefabNameSheetController* pns = [[PrefabNameSheetController alloc] init];
    NSWindow* prefabNameSheet = [pns window];
    
    NSApplication* app = [NSApplication sharedApplication];
    [app beginSheet:prefabNameSheet modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(prefabNameSheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
}

- (void)insertPrefab:(id <Prefab>)prefab {
    SelectionManager* selectionManager = [self selectionManager];
    [selectionManager removeAll:YES];
    
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    TVector3f insertPos = [camera defaultPoint];
    [[options grid] snapToGridV3f:&insertPos result:&insertPos];

    TVector3f offset;
    [[options grid] gridOffsetV3f:[prefab center] result:&offset];
    
    addV3f(&insertPos, &offset, &insertPos);

    TVector3f delta;
    subV3f(&insertPos, [prefab center], &delta);
    
    NSMutableArray* newEntities = [[NSMutableArray alloc] init];
    NSMutableArray* newBrushes = [[NSMutableArray alloc] init];
    MapDocument* map = [self document];
    
    for (id <Entity> prefabEntity in [prefab entities]) {
        id <Entity> mapEntity;
        if ([prefabEntity isWorldspawn]) {
            mapEntity = [map worldspawn:YES];
        } else {
            mapEntity = [map createEntityWithProperties:[prefabEntity properties]];
            [map setEntityDefinition:mapEntity];
            [newEntities addObject:mapEntity];
        }
        
        for (id <Brush> prefabBrush in [prefabEntity brushes]) {
            id <Brush> mapBrush = [map createBrushInEntity:mapEntity fromTemplate:prefabBrush];
            [newBrushes addObject:mapBrush];
        }
    }
    
    [[self document] translateEntities:newEntities delta:delta];
    [[self document] translateBrushes:newBrushes delta:delta lockTextures:[options lockTextures]];
    
    [selectionManager removeAll:YES];
    [selectionManager addEntities:newEntities record:YES];
    [selectionManager addBrushes:newBrushes record:YES];
    
    [newEntities release];
    [newBrushes release];

    [undoManager endUndoGrouping];
    [undoManager setActionName:[NSString stringWithFormat:@"Insert Prefab '%@'", [prefab name]]];
}

#pragma mark -
#pragma mark Compile & Run

- (IBAction)compile:(id)sender {
    NSMenuItem* item = (NSMenuItem *)sender;
    int index = [item tag];

    CompilerProfileManager* profileManager = [CompilerProfileManager sharedManager];
    if (index < 0 && index >= [[profileManager profiles] count])
        return;
    
    MapDocument* map = [self document];
    [map saveDocument:self];
    
    CompilerProfile* profile = [[profileManager profiles] objectAtIndex:index];
    NSURL* mapFileUrl = [map fileURL];
    
    MapCompiler* compiler = [[MapCompiler alloc] initWithMapFileUrl:mapFileUrl profile:profile console:console];
    [[console window] makeKeyAndOrderFront:self];
    [compiler compile];
    [compiler release];
    
    [[PreferencesManager sharedManager] setLastCompilerProfileIndex:index];
}

- (IBAction)run:(id)sender {
    NSMenuItem* menuItem = (NSMenuItem *)sender;
    int index = [menuItem tag];
    
    NSString* appPath = [[[PreferencesManager sharedManager] availableExecutables] objectAtIndex:index];
    [self runQuake:appPath];
}

- (IBAction)compileMostRecent:(id)sender {
    int index = [[PreferencesManager sharedManager] lastCompilerProfileIndex];
    CompilerProfileManager* profileManager = [CompilerProfileManager sharedManager];
    if (index < 0 && index >= [[profileManager profiles] count])
        return;
    
    MapDocument* map = [self document];
    [map saveDocument:self];
    
    CompilerProfile* profile = [[profileManager profiles] objectAtIndex:index];
    NSURL* mapFileUrl = [map fileURL];
    
    MapCompiler* compiler = [[MapCompiler alloc] initWithMapFileUrl:mapFileUrl profile:profile console:console];
    [[console window] makeKeyAndOrderFront:self];
    [compiler compile];
    [compiler release];
}

- (IBAction)runDefaultEngine:(id)sender {
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSString* quakePath = [preferences quakePath];
    if (quakePath == nil)
        return;
    NSString* quakeExecutable = [preferences quakeExecutable];
    if (quakeExecutable == nil)
        return;
    NSString* appPath = [quakePath stringByAppendingPathComponent:quakeExecutable];
    appPath = [appPath stringByAppendingPathExtension:@"app"];
    
    [self runQuake:appPath];
}

- (void)makeEntityVisible:(id <Entity>)theEntity {
    SelectionManager* selectionManager = [self selectionManager];
    [selectionManager removeAll:NO];
    [selectionManager addEntity:theEntity record:NO];
    [selectionManager addBrushes:[theEntity brushes] record:NO];
    
    [options setIsolationMode:IM_WIREFRAME];
    
    TVector3f center, size;
    TBoundingBox bounds;
    [selectionManager selectionBounds:&bounds];
    [selectionManager selectionCenter:&center];
    sizeOfBounds(&bounds, &size);
    float l = fmaxf(size.x, fmaxf(size.y, size.z));
    
    if (l == 0)
        return;
    
    TVector3f position = center;
    position.x -= l;
    position.y -= l;
    position.z += l;
    
    CameraAbsoluteAnimation* animation = [[CameraAbsoluteAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

- (void)makeBrushVisible:(id <Brush>)theBrush {
    SelectionManager* selectionManager = [self selectionManager];
    [selectionManager removeAll:NO];
    [selectionManager addBrush:theBrush record:NO];
    [options setIsolationMode:IM_WIREFRAME];
    
    TVector3f center, size;
    TBoundingBox bounds;
    [selectionManager selectionBounds:&bounds];
    [selectionManager selectionCenter:&center];
    sizeOfBounds(&bounds, &size);
    float l = fmaxf(size.x, fmaxf(size.y, size.z));
    
    TVector3f position = center;
    position.x -= l;
    position.y -= l;
    position.z += l;
    
    CameraAbsoluteAnimation* animation = [[CameraAbsoluteAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

- (void)makeFaceVisible:(id <Face>)theFace {
    SelectionManager* selectionManager = [self selectionManager];
    [selectionManager removeAll:NO];
    [selectionManager addBrush:[theFace brush] record:NO];
    [options setIsolationMode:IM_WIREFRAME];
    
    TVector3f position, direction, center;
    scaleV3f([theFace norm], 150, &position);
    centerOfVertices([theFace vertices], &center);
    addV3f(&center, &position, &position);
    scaleV3f([theFace norm], -1, &direction);
    
    CameraAbsoluteAnimation* animation = [[CameraAbsoluteAnimation alloc] initWithCamera:camera targetPosition:&position targetLookAt:&center duration:0.5];
    [animation startAnimation];
}

#pragma mark -
#pragma mark Getters

- (Camera *)camera {
    return camera;
}

- (SelectionManager *)selectionManager {
    return [[self document] selectionManager];
}

- (InputManager *)inputManager {
    return inputManager;
}

- (Options *)options {
    return options;
}

- (Renderer *)renderer {
    return [view3D renderer];
}

- (MapView3D *)view3D {
    return view3D;
}

- (ConsoleWindowController *)console {
    return console;
}

- (PreferencesManager *)preferences {
    return [PreferencesManager sharedManager];
}

@end

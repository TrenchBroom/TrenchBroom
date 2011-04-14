//
//  MapWindowController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MapWindowController.h"
#import "MapView3D.h"
#import "TextureView.h"
#import "MapDocument.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"
#import "Camera.h"
#import "MapDocument.h"
#import "WadLoader.h"
#import "Wad.h"
#import "TextureManager.h"
#import "InputManager.h"
#import "VBOBuffer.h"
#import "Octree.h"
#import "Picker.h"
#import "SelectionManager.h"
#import "GLFontManager.h"
#import "InspectorController.h"
#import "Options.h"
#import "Grid.h"
#import "Ray3D.h"
#import "Vector3f.h"
#import "Vector3i.h"
#import "PrefabManager.h"
#import "PrefabNameSheetController.h"
#import "Prefab.h"
#import "MapWriter.h"
#import "CameraAnimation.h"
#import "TrackingManager.h"
#import "CursorManager.h"
#import "ClipTool.h"

static NSString* CameraDefaults = @"Camera";
static NSString* CameraDefaultsFov = @"Field Of Vision";
static NSString* CameraDefaultsNear = @"Near Clipping Plane";
static NSString* CameraDefaultsFar = @"Far Clipping Plane";

@implementation MapWindowController

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self document] undoManager];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    InspectorController* inspector = [InspectorController sharedInspector];
    [inspector setMapWindowController:self];
}

- (void)userDefaultsChanged:(NSNotification *)notification {
    NSDictionary* cameraDefaults = [[NSUserDefaults standardUserDefaults] dictionaryForKey:CameraDefaults];
    if (cameraDefaults == nil)
        return;
    
    float fov = [[cameraDefaults objectForKey:CameraDefaultsFov] floatValue];
    float near = [[cameraDefaults objectForKey:CameraDefaultsNear] floatValue];
    float far = [[cameraDefaults objectForKey:CameraDefaultsFar] floatValue];
    
    [camera setFieldOfVision:fov];
    [camera setNearClippingPlane:near];
    [camera setFarClippingPlane:far];
}

- (void)windowDidLoad {
    GLResources* glResources = [[self document] glResources];
    NSOpenGLContext* sharedContext = [glResources openGLContext];
    NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[view3D pixelFormat] shareContext:sharedContext];
    [view3D setOpenGLContext:sharingContext];
    [sharingContext release];
    
    options = [[Options alloc] init];
    camera = [[Camera alloc] init];
    [self userDefaultsChanged:nil];
    
    selectionManager = [[SelectionManager alloc] initWithUndoManager:[[self document] undoManager]];
    trackingManager = [[TrackingManager alloc] initWithWindowController:self];
    inputManager = [[InputManager alloc] initWithWindowController:self];
    cursorManager = [[CursorManager alloc] init];
    
    [view3D setup];
    
    InspectorController* inspector = [InspectorController sharedInspector];
    [inspector setMapWindowController:self];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:[self window]];
    [center addObserver:self selector:@selector(userDefaultsChanged:) name:NSUserDefaultsDidChangeNotification object:[NSUserDefaults standardUserDefaults]];
    
    [[self window] setAcceptsMouseMovedEvents:YES];
    [[self window] makeKeyAndOrderFront:nil];
}

- (Camera *)camera {
    return camera;
}

- (SelectionManager *)selectionManager {
    return selectionManager;
}

- (InputManager *)inputManager {
    return inputManager;
}

- (TrackingManager *)trackingManager {
    return trackingManager;
}

- (CursorManager *)cursorManager {
    return cursorManager;
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

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    SEL action = [menuItem action];
    if (action == @selector(clearSelection:)) {
        return [selectionManager hasSelection];
    } else if (action == @selector(copySelection:)) {
        return [selectionManager hasSelectedEntities] || [selectionManager hasSelectedBrushes];
    } else if (action == @selector(cutSelection:)) {
        return [selectionManager hasSelectedEntities] || [selectionManager hasSelectedBrushes];
    } else if (action == @selector(pasteClipboard:)) {
        return NO;
    } else if (action == @selector(deleteSelection:)) {
        return [selectionManager hasSelectedEntities] || [selectionManager hasSelectedBrushes] || [clipTool numPoints] > 0;
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
    } else if (action == @selector(duplicateSelection:)) {
        return [selectionManager hasSelectedBrushes] && clipTool == nil;
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
        return [selectionManager hasSelectedBrushes];
    } else if (action == @selector(toggleClipMode:)) {
        return clipTool != nil;
    } else if (action == @selector(performClip:)) {
        return clipTool != nil && [clipTool numPoints] > 1;
    } else if (action == @selector(rotateZ90CW:)) {
        return [selectionManager hasSelectedBrushes];
    } else if (action == @selector(rotateZ90CCW:)) {
        return [selectionManager hasSelectedBrushes];
    }

    return NO;
}

- (IBAction)showInspector:(id)sender {
    InspectorController* inspector = [InspectorController sharedInspector];
    [[inspector window] makeKeyAndOrderFront:nil];
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
    Vector3f* center = [[selectionManager selectionCenter] retain];
    if (center == nil) {
        center = [[Vector3f alloc] initWithFloatVector:[camera direction]];
        [center scale:256];
        [center add:[camera position]];
    }
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:center];
    [diff sub:[camera position]];
    
    Vector3f* position = [[Vector3f alloc] initWithFloatVector:center];
    [position setZ:[position z] + [diff length]];
    
    CameraAnimation* animation = [[CameraAnimation alloc] initWithCamera:camera targetPosition:position targetDirection:[Vector3f zAxisNeg] targetUp:[Vector3f yAxisPos] duration:0.5];
    [animation startAnimation];
    
    [diff release];
    [position release];
    [center release];
}

- (IBAction)switchToXZView:(id)sender {
    Vector3f* center = [[selectionManager selectionCenter] retain];
    if (center == nil) {
        center = [[Vector3f alloc] initWithFloatVector:[camera direction]];
        [center scale:256];
        [center add:[camera position]];
    }
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:center];
    [diff sub:[camera position]];
    
    Vector3f* position = [[Vector3f alloc] initWithFloatVector:center];
    [position setY:[position y] - [diff length]];
    
    CameraAnimation* animation = [[CameraAnimation alloc] initWithCamera:camera targetPosition:position targetDirection:[Vector3f yAxisPos] targetUp:[Vector3f zAxisPos] duration:0.5];
    [animation startAnimation];

    [diff release];
    [position release];
    [center release];
}

- (IBAction)switchToYZView:(id)sender {
    Vector3f* center = [[selectionManager selectionCenter] retain];
    if (center == nil) {
        center = [[Vector3f alloc] initWithFloatVector:[camera direction]];
        [center scale:256];
        [center add:[camera position]];
    }
    
    Vector3f* diff = [[Vector3f alloc] initWithFloatVector:center];
    [diff sub:[camera position]];
    
    Vector3f* position = [[Vector3f alloc] initWithFloatVector:center];
    [position setX:[position x] + [diff length]];
    
    CameraAnimation* animation = [[CameraAnimation alloc] initWithCamera:camera targetPosition:position targetDirection:[Vector3f xAxisNeg] targetUp:[Vector3f zAxisPos] duration:0.5];
    [animation startAnimation];
    
    [diff release];
    [position release];
    [center release];
}

- (IBAction)clearSelection:(id)sender {
    [selectionManager removeAll:NO];
}

- (IBAction)copySelection:(id)sender {}
- (IBAction)cutSelection:(id)sender {}
- (IBAction)pasteClipboard:(id)sender {}

- (IBAction)deleteSelection:(id)sender {
    if (clipTool != nil) {
        [clipTool deleteLastPoint];
    } else {
        NSUndoManager* undoManager = [[self document] undoManager];
        [undoManager beginUndoGrouping];
        
        NSSet* deletedBrushes = [[NSSet alloc] initWithSet:[selectionManager selectedBrushes]];
        [selectionManager removeAll:YES];
        
        NSEnumerator* brushEn = [deletedBrushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject]))
            [[self document] deleteBrush:brush];
        
        [deletedBrushes release];
        
        [undoManager endUndoGrouping];
        [undoManager setActionName:@"Delete Selection"];
    }
}

- (IBAction)moveTextureLeft:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    int d = ![[options grid] snap] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [[options grid] actualSize];

    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] translateFaceOffset:face xDelta:d yDelta:0];

    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Texture"];
}

- (IBAction)moveTextureRight:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    int d = ![[options grid] snap] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [[options grid] actualSize];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] translateFaceOffset:face xDelta:-d yDelta:0];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Texture"];
}

- (IBAction)moveTextureUp:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    int d = ![[options grid] snap] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [[options grid] actualSize];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] translateFaceOffset:face xDelta:0 yDelta:d];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Texture"];
}

- (IBAction)moveTextureDown:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    int d = ![[options grid] snap] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [[options grid] actualSize];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] translateFaceOffset:face xDelta:0 yDelta:-d];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Texture"];
}

- (IBAction)stretchTextureHorizontally:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] setFace:face xScale:[face xScale] + 0.1f];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Stretch Texture Horizontally"];
}

- (IBAction)shrinkTextureHorizontally:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] setFace:face xScale:[face xScale] - 0.1f];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Shrink Texture Horizontally"];
}

- (IBAction)stretchTextureVertically:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] setFace:face yScale:[face yScale] + 0.1f];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Stretch Texture Vertically"];
}

- (IBAction)shrinkTextureVertically:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] setFace:face yScale:[face yScale] - 0.1f];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Shrink Texture Vertically"];
}

- (IBAction)rotateTextureLeft:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    int d = ![[options grid] snap] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : 15;
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] setFace:face rotation:[face rotation] - d];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Rotate Texture Left"];
}

- (IBAction)rotateTextureRight:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    int d = ![[options grid] snap] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : 15;
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] setFace:face rotation:[face rotation] + d];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Rotate Texture Right"];
}

- (IBAction)duplicateSelection:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    id <Entity> worldspawn = [[self document] worldspawn:YES];
    NSMutableSet* newBrushes = [[NSMutableSet alloc] init];

    NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Brush> newBrush = [[self document] createBrushInEntity:worldspawn fromTemplate:brush];
        [[self document] translateBrush:newBrush xDelta:[[options grid] actualSize] yDelta:[[options grid] actualSize] zDelta:0];
        [newBrushes addObject:newBrush];
    }
    
    [selectionManager removeAll:YES];
    [selectionManager addBrushes:newBrushes record:YES];
    [newBrushes release];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Duplicate Selection"];
}

- (void)prefabNameSheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo {
    PrefabNameSheetController* pns = [sheet windowController];
    if (returnCode == NSOKButton) {
        NSString* prefabName = [pns prefabName];
        NSString* prefabGroupName = [pns prefabGroup];
        
        PrefabManager* prefabManager = [PrefabManager sharedPrefabManager];
        id <PrefabGroup> prefabGroup = [prefabManager prefabGroupWithName:prefabGroupName create:YES];
        [prefabManager createPrefabFromBrushTemplates:[selectionManager selectedBrushes] name:prefabName group:prefabGroup];
    }
    [pns release];
}

- (IBAction)createPrefabFromSelection:(id)sender {
    PrefabNameSheetController* pns = [[PrefabNameSheetController alloc] init];
    NSWindow* prefabNameSheet = [pns window];
    
    NSApplication* app = [NSApplication sharedApplication];
    [app beginSheet:prefabNameSheet modalForWindow:[self window] modalDelegate:self didEndSelector:@selector(prefabNameSheetDidEnd:returnCode:contextInfo:) contextInfo:nil];
    
}

- (IBAction)toggleClipTool:(id)sender {
    /*
    if (clipTool == nil) {
        clipTool = [[ClipTool alloc] initWithBrushes:[selectionManager selectedBrushes] picker:[[self document] picker] grid:[options grid] renderer:[view3D renderer]];
    } else {
        [clipTool release];
        clipTool = nil;
    }
    
    [inputManager setClipTool:clipTool];
     */
}

- (IBAction)toggleClipMode:(id)sender {
    [clipTool toggleClipMode];
}

- (IBAction)performClip:(id)sender {
    /*
    [selectionManager removeAll:YES];
    NSSet* newBrushes = [clipTool performClip:[self document]];
    [inputManager setClipTool:nil];
    [clipTool release];
    clipTool = nil;
    
    [selectionManager addBrushes:newBrushes record:YES];
     */
}

- (IBAction)rotateZ90CW:(id)sender {
    [[self document] rotateZ90CW:[selectionManager selectedBrushes]];
}

- (IBAction)rotateZ90CCW:(id)sender {
    [[self document] rotateZ90CCW:[selectionManager selectedBrushes]];
}

- (void)insertPrefab:(id <Prefab>)prefab {
    [selectionManager removeAll:YES];
    
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    Vector3f* insertPos = [camera defaultPoint];
    [[options grid] snapToGrid:insertPos];
    
    Vector3f* offset = [[options grid] gridOffsetOf:[prefab center]];
    [insertPos add:offset];

    Vector3f* dist = [[Vector3f alloc] initWithFloatVector:insertPos];
    [dist sub:[prefab center]];
    
    MapDocument* map = [self document];
    
    NSEnumerator* entityEn = [[prefab entities] objectEnumerator];
    id <Entity> prefabEntity;
    while ((prefabEntity = [entityEn nextObject])) {
        id <Entity> mapEntity;
        if ([prefabEntity isWorldspawn]) {
            mapEntity = [map worldspawn:YES];
        } else {
            mapEntity = [map createEntityWithProperties:[prefabEntity properties]];
        }
        
        NSEnumerator* prefabBrushEn = [[prefabEntity brushes] objectEnumerator];
        id <Brush> prefabBrush;
        while ((prefabBrush = [prefabBrushEn nextObject])) {
            id <Brush> mapBrush = [map createBrushInEntity:mapEntity fromTemplate:prefabBrush];
            [map translateBrush:mapBrush xDelta:roundf([dist x]) yDelta:roundf([dist y]) zDelta:roundf([dist z])];
            [selectionManager addBrush:mapBrush record:YES];
        }
    }
    
    
    [dist release];

    [undoManager endUndoGrouping];
    [undoManager setActionName:[NSString stringWithFormat:@"Insert Prefab '%@'", [prefab name]]];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [options release];
    [trackingManager release];
    [selectionManager release];
    [inputManager release];
    [cursorManager release];
    [camera release];
    [super dealloc];
}

@end

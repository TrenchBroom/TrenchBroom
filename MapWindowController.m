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
#import "Ray3D.h"
#import "Vector3f.h"
#import "Vector3i.h"

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
    [camera setFarCliippingPlane:far];
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
    
    selectionManager = [[SelectionManager alloc] init];
    inputManager = [[InputManager alloc] initWithWindowController:self];
    
    [view3D setup];
    
    InspectorController* inspector = [InspectorController sharedInspector];
    [inspector setMapWindowController:self];
    [inspector showWindow:nil];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:[self window]];
    [center addObserver:self selector:@selector(userDefaultsChanged:) name:NSUserDefaultsDidChangeNotification object:[NSUserDefaults standardUserDefaults]];
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

- (Options *)options {
    return options;
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
        return [selectionManager hasSelectedEntities] || [selectionManager hasSelectedBrushes];
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
        return [selectionManager hasSelectedBrushes];
    } else if (action == @selector(createPrefabFromSelection:)) {
        return [selectionManager hasSelectedBrushes];
    }

    return NO;
}

- (IBAction)toggleGrid:(id)sender {
    [options setDrawGrid:![options drawGrid]];
}

- (IBAction)toggleSnap:(id)sender {
    [options setSnapToGrid:![options snapToGrid]];
}

- (IBAction)setGridSize:(id)sender {
    [options setGridSize:[sender tag]];
}

- (IBAction)clearSelection:(id)sender {
    [selectionManager removeAll];
}

- (IBAction)copySelection:(id)sender {}
- (IBAction)cutSelection:(id)sender {}
- (IBAction)pasteClipboard:(id)sender {}

- (IBAction)deleteSelection:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    NSSet* deletedBrushes = [[NSSet alloc] initWithSet:[selectionManager selectedBrushes]];

    NSEnumerator* brushEn = [deletedBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [[self document] deleteBrush:brush];
    
    [selectionManager removeAll];
    [deletedBrushes release];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Delete Selection"];
}

- (IBAction)moveTextureLeft:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];
    
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [options gridSize];

    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] translateFaceOffset:face xDelta:-d yDelta:0];

    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Texture"];
}

- (IBAction)moveTextureRight:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [options gridSize];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject]))
        [[self document] translateFaceOffset:face xDelta:d yDelta:0];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Move Texture"];
}

- (IBAction)moveTextureUp:(id)sender {
    NSUndoManager* undoManager = [[self document] undoManager];
    [undoManager beginUndoGrouping];

    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [options gridSize];
    
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

    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [options gridSize];
    
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
    
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : 15;
    
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
    
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : 15;
    
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
    
    id <Entity> worldspawn = [[self document] worldspawn];
    NSMutableSet* newBrushes = [[NSMutableSet alloc] init];

    NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        id <Brush> newBrush = [[self document] createBrushInEntity:worldspawn fromTemplate:brush];
        [[self document] translateBrush:newBrush xDelta:[options gridSize] yDelta:[options gridSize] zDelta:[options gridSize]];
        [newBrushes addObject:newBrush];
    }
    
    [[undoManager prepareWithInvocationTarget:selectionManager] addBrushes:[NSSet setWithSet:[selectionManager selectedBrushes]]];
    [[undoManager prepareWithInvocationTarget:selectionManager] removeAll];
    
    [selectionManager removeAll];
    [selectionManager addBrushes:newBrushes];
    [newBrushes release];
    
    [undoManager endUndoGrouping];
    [undoManager setActionName:@"Duplicate Selection"];
}

- (IBAction)createPrefabFromSelection:(id)sender {
}

- (void)insertPrefab:(Prefab *)prefab {
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [options release];
    [selectionManager release];
    [inputManager release];
    [camera release];
    [super dealloc];
}

@end

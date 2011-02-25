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
#import "Map.h"
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
#import "ToolManager.h"
#import "Options.h"

@implementation MapWindowController

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self document] undoManager];
}

- (void)windowDidLoad {
    GLResources* glResources = [[self document] glResources];
    NSOpenGLContext* sharedContext = [glResources openGLContext];
    NSOpenGLContext* sharingContext = [[NSOpenGLContext alloc] initWithFormat:[view3D pixelFormat] shareContext:sharedContext];
    [view3D setOpenGLContext:sharingContext];
    [sharingContext release];
    
    options = [[Options alloc] init];
    camera = [[Camera alloc] init];
    
    selectionManager = [[SelectionManager alloc] init];
    inputManager = [[InputManager alloc] initWithWindowController:self];
    
    [view3D setup];
    
    InspectorController* inspector = [InspectorController sharedInspector];
    [inspector setMapWindowController:self];
    [inspector showWindow:nil];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:[self window]];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    InspectorController* inspector = [InspectorController sharedInspector];
    [inspector setMapWindowController:self];
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

- (IBAction)deleteSelection:(id)sender {}

- (IBAction)moveTextureLeft:(id)sender {
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [options gridSize];

    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face translateOffsetsX:-d y:0];
}

- (IBAction)moveTextureRight:(id)sender {
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [options gridSize];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face translateOffsetsX:d y:0];
}

- (IBAction)moveTextureUp:(id)sender {
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [options gridSize];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face translateOffsetsX:0 y:d];
}

- (IBAction)moveTextureDown:(id)sender {
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : [options gridSize];
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face translateOffsetsX:0 y:-d];
}

- (IBAction)stretchTextureHorizontally:(id)sender {
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setXScale:[face xScale] + 0.1f];
}

- (IBAction)shrinkTextureHorizontally:(id)sender {
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setXScale:[face xScale] - 0.1f];
}

- (IBAction)stretchTextureVertically:(id)sender {
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setYScale:[face yScale] + 0.1f];
}

- (IBAction)shrinkTextureVertically:(id)sender {
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setYScale:[face yScale] - 0.1f];
}

- (IBAction)rotateTextureLeft:(id)sender {
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : 15;
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setRotation:[face rotation] - d];
}

- (IBAction)rotateTextureRight:(id)sender {
    int d = ![options snapToGrid] ^ ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0 ? 1 : 15;
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setRotation:[face rotation] + d];
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

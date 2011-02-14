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
#import "FaceInspectorController.h"
#import "ToolManager.h"
#import "Options.h"

@implementation MapWindowController

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self document] undoManager];
}

- (void)windowDidLoad {
    NSOpenGLContext* glContext = [view3D openGLContext];
    [glContext makeCurrentContext];
    
    options = [[Options alloc] init];
    
    Map* map = [[self document] map];
    
    NSBundle* mainBundle = [NSBundle mainBundle];
    NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
    NSData* palette = [[NSData alloc] initWithContentsOfFile:palettePath];
    
    textureManager = [[TextureManager alloc] initWithPalette:palette];
    [palette release];

    NSString* wads = [[map worldspawn] propertyForKey:@"wad"];
    if (wads != nil) {
        NSArray* wadPaths = [wads componentsSeparatedByString:@";"];
        for (int i = 0; i < [wadPaths count]; i++) {
            NSString* wadPath = [[wadPaths objectAtIndex:i] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            NSFileManager* fileManager = [NSFileManager defaultManager];
            if ([fileManager fileExistsAtPath:wadPath]) {
                int slashIndex = [wadPath rangeOfString:@"/" options:NSBackwardsSearch].location;
                NSString* wadName = [wadPath substringFromIndex:slashIndex + 1];
                
                WadLoader* wadLoader = [[WadLoader alloc] init];
                Wad* wad = [wadLoader loadFromData:[NSData dataWithContentsOfMappedFile:wadPath] wadName:wadName];
                [wadLoader release];
                
                [textureManager loadTexturesFrom:wad];
            }
        }
    }
    
    fontManager = [[GLFontManager alloc] init];
    
    octree = [[Octree alloc] initWithMap:map minSize:64];
    picker = [[Picker alloc] initWithOctree:octree];
    camera = [[Camera alloc] init];
    
    selectionManager = [[SelectionManager alloc] init];
    toolManager = [[ToolManager alloc] initWithSelectionManager:selectionManager undoManager:[[self document] undoManager] options:options];
    inputManager = [[InputManager alloc] initWithPicker:picker selectionManager:selectionManager toolManager:toolManager];
    
    vbo = [[VBOBuffer alloc] initWithTotalCapacity:8192];
    [view3D setup];
    
    FaceInspectorController* faceInspector = [FaceInspectorController sharedInspector];
    [[faceInspector window] makeKeyAndOrderFront:self];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:[self window]];
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    FaceInspectorController* inspector = [FaceInspectorController sharedInspector];
    [inspector switchToContext:[view3D openGLContext] 
              selectionManager:selectionManager 
                textureManager:textureManager];
}

- (void)windowWillClose:(NSNotification *)notification {
    [textureManager disposeTextures];
    [fontManager dispose];
    [vbo dispose];
}

- (VBOBuffer *)vbo {
    return vbo;
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

- (TextureManager *)textureManager {
    return textureManager;
}

- (ToolManager *)toolManager {
    return toolManager;
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

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [options release];
    [toolManager release];
    [vbo release];
    [selectionManager release];
    [picker release];
    [octree release];
    [inputManager release];
    [textureManager release];
    [camera release];
    [super dealloc];
}

@end

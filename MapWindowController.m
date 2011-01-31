//
//  MapWindowController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MapWindowController.h"
#import "MapView2D.h"
#import "MapView3D.h"
#import "TextureView.h"
#import "RenderMap.h"
#import "Map.h"
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

@implementation MapWindowController

- (void)windowDidLoad {
    faceVBO = [[VBOBuffer alloc] initWithTotalCapacity:8192];
    edgeVBO = [[VBOBuffer alloc] initWithTotalCapacity:8192];

    Map* map = [[self document] map];
    renderMap = [[RenderMap alloc] initWithMap:map faceVBO:faceVBO edgeVBO:edgeVBO];
    camera = [[Camera alloc] init];
    
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
    
    octree = [[Octree alloc] initWithMap:map minSize:64];
    picker = [[Picker alloc] initWithOctree:octree];
    
    selectionManager = [[SelectionManager alloc] init];
    inputManager = [[InputManager alloc] initWithPicker:picker selectionManager:selectionManager];
    
    [textureView setTextureManager:textureManager];
    [view3D setTextureManager:textureManager];
    [view3D setInputManager:inputManager];
    [view3D setSelectionManager:selectionManager];
    [view3D setFaceVBO:faceVBO];
    [view3D setEdgeVBO:edgeVBO];
    [view3D setCamera:camera];
    [view3D setRenderMap:renderMap];
    
}

- (void)windowWillClose:(NSNotification *)notification {
    [textureManager disposeTextures];
    [faceVBO dispose];
    [edgeVBO dispose];
}

- (void)dealloc {
    [faceVBO release];
    [edgeVBO release];
    [selectionManager release];
    [picker release];
    [octree release];
    [inputManager release];
    [textureManager release];
    [renderMap release];
    [camera release];
    [super dealloc];
}

@end

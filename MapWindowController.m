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
#import "TextureWindowController.h"
#import "GLFontManager.h"
#import "SingleTextureView.h"

@implementation MapWindowController

- (void)windowDidLoad {
    NSOpenGLContext* glContext = [view3D openGLContext];
    [glContext makeCurrentContext];
    
    NSOpenGLContext* textureViewContext = [[NSOpenGLContext alloc] initWithFormat:[textureView pixelFormat] shareContext:glContext];
    [textureView setOpenGLContext:textureViewContext];
    [textureViewContext release];
    
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

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(selectionChanged:) name:SelectionChanged object:selectionManager];
    
    inputManager = [[InputManager alloc] initWithPicker:picker selectionManager:selectionManager];
    
    faceVBO = [[VBOBuffer alloc] initWithTotalCapacity:8192];
    renderMap = [[RenderMap alloc] initWithMap:map faceVBO:faceVBO camera:camera textureManager:textureManager selectionManager:selectionManager];

    [view3D setTextureManager:textureManager];
    [view3D setInputManager:inputManager];
    [view3D setSelectionManager:selectionManager];
    [view3D setFaceVBO:faceVBO];
    [view3D setCamera:camera];
    [view3D setRenderMap:renderMap];
 
    TextureWindowController* textureWindowController = [[TextureWindowController alloc] initWithWindowNibName:@"TextureBrowser" sharedContext:glContext textureManager:textureManager fontManager:fontManager];
    [[textureWindowController window] makeKeyAndOrderFront:self];
    
    [[self window] makeKeyAndOrderFront:self];
    [faceInspector makeKeyAndOrderFront:self];
}

- (void)selectionChanged:(NSNotification *)notification {
    if ([selectionManager mode] == SM_FACES) {
        [xOffsetField setEnabled:YES];
        [yOffsetField setEnabled:YES];
        [xScaleField setEnabled:YES];
        [yScaleField setEnabled:YES];
        [rotationField setEnabled:YES];

        NSSet* faces = [selectionManager selectedFaces];
        NSEnumerator* faceEn = [faces objectEnumerator];
        Face* face = [faceEn nextObject];
        
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
            [xOffsetField setStringValue:[NSString stringWithFormat:@"%i", xOffset]];
        }
        
        if (yOffsetMultiple) {
            [[yOffsetField cell] setPlaceholderString:@"multiple"];
            [yOffsetField setStringValue:@""];
        } else {
            [yOffsetField setStringValue:[NSString stringWithFormat:@"%i", yOffset]];
        }
        
        if (xScaleMultiple) {
            [[xScaleField cell] setPlaceholderString:@"multiple"];
            [xScaleField setStringValue:@""];
        } else {
            [xScaleField setStringValue:[NSString stringWithFormat:@"%f", xScale]];
        }
        
        if (yScaleMultiple) {
            [[yScaleField cell] setPlaceholderString:@"multiple"];
            [yScaleField setStringValue:@""];
        } else {
            [yScaleField setStringValue:[NSString stringWithFormat:@"%f", yScale]];
        }
        
        if (rotationMultiple) {
            [[rotationField cell] setPlaceholderString:@"multiple"];
            [rotationField setStringValue:@""];
        } else {
            [rotationField setStringValue:[NSString stringWithFormat:@"%f", rotation]];
        }
        
        if (textureMultiple) {
            [[textureNameField cell] setPlaceholderString:@"multiple"];
            [textureNameField setStringValue:@""];
            [textureView setTexture:nil];
        } else {
            [textureNameField setStringValue:textureName];
            
            Texture* texture = [textureManager textureForName:textureName];
            [textureView setTexture:texture];
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
        [textureView setTexture:nil];
    }
    
}

- (void)windowWillClose:(NSNotification *)notification {
    [textureManager disposeTextures];
    [fontManager dispose];
    [faceVBO dispose];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [faceVBO release];
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

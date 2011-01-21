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
#import "RenderMap.h"
#import "Map.h"
#import "Camera.h"
#import "MapDocument.h"
#import "WadLoader.h"
#import "Wad.h"
#import "TextureManager.h"

@implementation MapWindowController

- (void)windowDidLoad {
    Map* map = [[self document] map];
    renderMap = [[RenderMap alloc] initWithMap:map];
    camera = [[Camera alloc] init];
    
    [view3D setCamera:camera];
    [view3D setRenderMap:renderMap];
    
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
}

- (void)windowWillClose:(NSNotification *)notification {
    [textureManager disposeTextures];
}

- (void)dealloc {
    [textureManager release];
    [renderMap release];
    [camera release];
    [super dealloc];
}

@end

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

@implementation MapWindowController

- (void)windowDidLoad {
    Map* map = [[self document] map];
    renderMap = [[RenderMap alloc] initWithMap:map];
    camera = [[Camera alloc] init];
    
    [view3D setCamera:camera];
    [view3D setRenderMap:renderMap];
}

- (void)dealloc {
    [renderMap release];
    [camera release];
    [super dealloc];
}

@end

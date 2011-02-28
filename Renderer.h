//
//  Renderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

extern NSString* const RendererChanged;

@class RenderContext;
@class MapWindowController;

@interface Renderer : NSObject {
    @private
    MapWindowController* windowController;
    id<Layer> geometryLayer;
    id<Layer> selectionLayer;
    NSMutableDictionary* faceFigures;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)render;
- (void)updateView:(NSRect)bounds;

@end

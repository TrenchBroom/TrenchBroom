//
//  Renderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"
#import "Layer.h"

extern NSString* const RendererChanged;

@class RenderContext;
@class MapWindowController;

@interface Renderer : Observable {
    @private
    MapWindowController* windowController;
    NSObject<Layer>* geometryLayer;
    NSObject<Layer>* selectionLayer;
    NSObject<Layer>* toolLayer;
    NSMutableDictionary* faceFigures;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)render;
- (void)updateView:(NSRect)bounds;

@end

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

@class GeometryLayer;
@class SelectionLayer;
@class FeedbackLayer;
@class RenderContext;
@class MapWindowController;

@interface Renderer : NSObject {
    @private
    MapWindowController* windowController;
    GeometryLayer* geometryLayer;
    SelectionLayer* selectionLayer;
    FeedbackLayer* feedbackLayer;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)render;

@end

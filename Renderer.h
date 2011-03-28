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

@class VBOBuffer;
@class GeometryLayer;
@class SelectionLayer;
@class TrackingLayer;
@class RenderContext;
@class MapWindowController;
@class TextureManager;
@protocol FeedbackFigure;

@interface Renderer : NSObject {
    @private
    MapWindowController* windowController;
    TextureManager* textureManager;
    VBOBuffer* sharedVbo;
    NSMutableSet* invalidFaces;
    NSMutableSet* feedbackFigures;
    GeometryLayer* geometryLayer;
    SelectionLayer* selectionLayer;
    TrackingLayer* trackingLayer;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)addFeedbackFigure:(id <FeedbackFigure>)theFigure;
- (void)removeFeedbackFigure:(id <FeedbackFigure>)theFigure;

- (void)render;

@end

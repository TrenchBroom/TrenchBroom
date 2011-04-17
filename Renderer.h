//
//  Renderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BrushLayer.h"

extern NSString* const RendererChanged;

@class VBOBuffer;
@class GeometryLayer;
@class SelectionLayer;
@class FigureLayer;
@class CompassFigure;
@class RenderContext;
@class MapWindowController;
@class TextureManager;
@protocol Figure;
@protocol Filter;

@interface Renderer : NSObject {
    @private
    MapWindowController* windowController;
    TextureManager* textureManager;
    VBOBuffer* sharedVbo;
    NSMutableSet* invalidFaces;
    GeometryLayer* geometryLayer;
    SelectionLayer* selectionLayer;
    FigureLayer* feedbackLayer;
    CompassFigure* compassFigure;
    id <Filter> filter;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)addFeedbackFigure:(id <Figure>)theFigure;
- (void)removeFeedbackFigure:(id <Figure>)theFigure;

- (void)setFilter:(id <Filter>)theFilter;
- (void)render;

@end

//
//  GeometryLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@class PolygonRenderer;
@class LineRenderer;
@class MapWindowController;
@class RenderContext;

@interface GeometryLayer : NSObject <Layer> {
    PolygonRenderer* faceRenderer;
    LineRenderer* edgeRenderer;
    MapWindowController* windowController;
}

- (id)initWithWindowController:(MapWindowController *)theMapWindowController;

- (void)renderTexturedFaces;
- (void)renderFlatFaces;
- (void)renderEdges;

- (void)render:(RenderContext *)renderContext;

@end

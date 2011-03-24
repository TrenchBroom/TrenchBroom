//
//  GeometryLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@class FaceRenderer;
@class MapWindowController;
@class RenderContext;
@protocol EdgeRenderer;

@interface GeometryLayer : NSObject <Layer> {
    FaceRenderer* faceRenderer;
    id <EdgeRenderer> edgeRenderer;
    MapWindowController* windowController;
}

- (id)initWithWindowController:(MapWindowController *)theMapWindowController;
- (id <EdgeRenderer>)createEdgeRenderer;

- (void)renderTexturedFaces;
- (void)renderFlatFaces;
- (void)renderEdges;

- (void)render:(RenderContext *)renderContext;

@end

//
//  GeometryLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@class MapWindowController;
@class RenderContext;

@interface GeometryLayer : NSObject <Layer> {
    NSMutableSet* faceFigures;
    NSMutableDictionary* indexBuffers;
    NSMutableDictionary* countBuffers;
    MapWindowController* mapWindowController;
    BOOL buffersValid;
}

- (id)initWithWindowController:(MapWindowController *)theMapWindowController;

- (void)renderWireframe:(RenderContext *)renderContext;
- (void)renderTextured:(RenderContext *)renderContext;
- (void)renderFaces:(RenderContext *)renderContext;
- (void)prepare:(RenderContext *)renderContext;
- (void)render:(RenderContext *)renderContext;

@end

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
@class IntData;

@interface GeometryLayer : NSObject <Layer> {
    NSMutableSet* faceFigures;
    NSMutableSet* edgeFigures;
    NSMutableDictionary* faceIndexBuffers;
    NSMutableDictionary* faceCountBuffers;
    NSNumber* edgeVboKey;
    MapWindowController* mapWindowController;
    BOOL buffersValid;
}

- (id)initWithWindowController:(MapWindowController *)theMapWindowController;

- (void)renderFaces;
- (void)renderEdges;
- (void)prepare;
- (void)render:(RenderContext *)renderContext;

@end

//
//  TrackingLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@class ThickEdgeRenderer;
@class VertexRenderer;
@class Edge;
@class Vertex;

@interface TrackingLayer : NSObject <Layer> {
    ThickEdgeRenderer* edgeRenderer;
    VertexRenderer* vertexRenderer;
}

- (void)addEdge:(Edge *)theEdge;
- (void)removeEdge:(Edge *)theEdge;

- (void)addVertex:(Vertex *)theVertex;
- (void)removeVertex:(Vertex *)theVertex;

@end

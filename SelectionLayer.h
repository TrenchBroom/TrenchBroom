//
//  SelectionLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GeometryLayer.h"

@class GridRenderer;
@class VertexRenderer;
@class Options;
@class Edge;
@class Vertex;
@protocol Brush;
@protocol Face;

@interface SelectionLayer : GeometryLayer {
    GridRenderer* gridRenderer;
    VertexRenderer* vertexRenderer;
    Options* options;
}

- (void)addBrushVertices:(id <Brush>)theBrush;
- (void)removeBrushVertices:(id <Brush>)theBrush;

- (void)addFaceVertices:(id <Face>)theFace;
- (void)removeFaceVertices:(id <Face>)theFace;

- (void)addEdgeVertices:(Edge *)theEdge;
- (void)removeEdgeVertices:(Edge *)theEdge;

- (void)addVertex:(Vertex *)theVertex;
- (void)removeVertex:(Vertex *)theVertex;

- (void)renderGrid;
- (void)renderVertices;

@end

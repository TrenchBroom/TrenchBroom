//
//  Layer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RenderContext;
@class Edge;
@protocol Brush;
@protocol Face;

@protocol Layer <NSObject>

- (void)render:(RenderContext *)renderContext;

- (void)addBrushFaces:(id <Brush>)theBrush;
- (void)removeBrushFaces:(id <Brush>)theBrush;

- (void)addBrushEdges:(id <Brush>)theBrush;
- (void)removeBrushEdges:(id <Brush>)theBrush;

- (void)addFace:(id <Face>)theFace;
- (void)removeFace:(id <Face>)theFace;

- (void)addFaceEdges:(id <Face>)theFace;
- (void)removeFaceEdges:(id <Face>)theFace;

- (void)addEdge:(Edge *)theEdge;
- (void)removeEdge:(Edge *)theEdge;

@end

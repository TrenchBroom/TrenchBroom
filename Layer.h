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

- (void)addBrush:(id <Brush>)theBrush;
- (void)removeBrush:(id <Brush>)theBrush;

- (void)addFace:(id <Face>)theFace includeEdges:(BOOL)includeEdges;
- (void)removeFace:(id <Face>)theFace includeEdges:(BOOL)includeEdges;

- (void)addEdge:(Edge *)theEdge;
- (void)removeEdge:(Edge *)theEdge;

@end

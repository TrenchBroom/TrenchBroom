//
//  EdgeRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Edge;
@protocol RenderFilter;

@protocol EdgeRenderer <NSObject>

- (void)addEdge:(Edge *)theEdge;
- (void)removeEdge:(Edge *)theEdge;

- (void)setFilter:(id <RenderFilter>)theFilter;
- (void)render;

- (void)invalidate;

@end

//
//  RenderFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 23.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Face;
@class Edge;

@protocol RenderFilter <NSObject>

- (BOOL)facePasses:(id <Face>)theFace;
- (BOOL)edgePasses:(Edge *)theEdge;

@end

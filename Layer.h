//
//  Layer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RenderContext;
@class FaceFigure;
@class EdgeFigure;

@protocol Layer <NSObject>

- (void)render:(RenderContext *)renderContext;

- (void)addFaceFigure:(FaceFigure *)theFigure;
- (void)removeFaceFigure:(FaceFigure *)theFigure;

- (void)addEdgeFigure:(EdgeFigure *)theFigure;
- (void)removeEdgeFigure:(EdgeFigure *)theFigure;

@end

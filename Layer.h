//
//  Layer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RenderContext;
@protocol Figure;

@protocol Layer <NSObject>

- (void)render:(RenderContext *)renderContext;

- (void)addFigure:(id <Figure>)theFigure;
- (void)removeFigure:(id <Figure>)theFigure;

- (void)invalidate;
@end

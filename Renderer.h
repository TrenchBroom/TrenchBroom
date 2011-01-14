//
//  Renderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 12.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Brush;
@class RenderContext;

@protocol Renderer

- (void)renderBrush:(Brush *)brush context:(RenderContext *)context;
- (void)renderEntity:(Entity *)entity context:(RenderContext *)context;
- (void)renderMap:(Map *)map context:(RenderContext *)context;

@end

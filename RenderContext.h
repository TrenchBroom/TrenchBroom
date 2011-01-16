//
//  RenderContext.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Camera;
@class RenderBrush;

@protocol RenderContext

- (void)preRender;
- (void)updateView:(NSRect)bounds withCamera:(Camera *)camera;
- (void)renderBrush:(RenderBrush *)renderBrush;
- (void)postRender;
@end

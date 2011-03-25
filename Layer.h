//
//  Layer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RenderContext;
@protocol Brush;
@protocol Face;

@protocol Layer <NSObject>

- (void)render:(RenderContext *)renderContext;

- (void)addBrushFaces:(id <Brush>)theBrush;
- (void)removeBrushFaces:(id <Brush>)theBrush;

- (void)addFace:(id <Face>)theFace;
- (void)removeFace:(id <Face>)theFace;

@end

//
//  BrushLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Layer.h"

@protocol Brush;
@protocol Face;
@protocol Filter;

@protocol BrushLayer <Layer>

- (void)addBrush:(id <Brush>)theBrush;
- (void)removeBrush:(id <Brush>)theBrush;

- (void)addFace:(id <Face>)theFace;
- (void)removeFace:(id <Face>)theFace;

- (void)setFilter:(id <Filter>)theFilter;

@end

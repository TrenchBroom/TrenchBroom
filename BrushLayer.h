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

- (void)addBrushes:(NSSet *)theBrushes;
- (void)removeBrushes:(NSSet *)theBrushes;

- (void)addBrush:(id <Brush>)theBrush;
- (void)removeBrush:(id <Brush>)theBrush;

- (void)addFaces:(NSSet *)theFaces;
- (void)removeFaces:(NSSet *)theFaces;

- (void)addFace:(id <Face>)theFace;
- (void)removeFace:(id <Face>)theFace;

- (void)setFilter:(id <Filter>)theFilter;

@end

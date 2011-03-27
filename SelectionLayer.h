//
//  SelectionLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GeometryLayer.h"

@class GridRenderer;
@class FaceHandleRenderer;

@interface SelectionLayer : GeometryLayer {
    GridRenderer* gridRenderer;
    FaceHandleRenderer* handleRenderer;
    BOOL drawGrid;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager gridSize:(int)theGridSize drawGrid:(BOOL)doDrawGrid;

- (void)setGridSize:(int)theGridSize;
- (void)setDrawGrid:(BOOL)doDrawGrid;

@end

//
//  Options.m
//  TrenchBroom
//
//  Created by Kristian Duske on 13.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Options.h"

NSString* const OptionsChanged = @"OptionsChanged";

@implementation Options

- (id)init {
    if (self = [super init]) {
        drawGrid = YES;
        snapToGrid = YES;
        gridSize = 16;
    }
    
    return self;
}

- (BOOL)drawGrid {
    return drawGrid;
}

- (BOOL)snapToGrid {
    return snapToGrid;
}

- (int)gridSize {
    return gridSize;
}

- (ERenderMode)renderMode {
    return renderMode;
}

- (void)setDrawGrid:(BOOL)doDrawGrid {
    if (drawGrid == doDrawGrid)
        return;
    
    drawGrid = doDrawGrid;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (void)setSnapToGrid:(BOOL)doSnapToGrid {
    if (snapToGrid == doSnapToGrid)
        return;
    
    snapToGrid = doSnapToGrid;

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (void)setGridSize:(int)theGridSize {
    if (gridSize == theGridSize)
        return;
    
    if (gridSize <= 0)
        [NSException raise:NSInvalidArgumentException format:@"grid size must be a positive integer"];
    
    gridSize = theGridSize;

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (void)setRenderMode:(ERenderMode)theRenderMode {
    if (renderMode == theRenderMode)
        return;
    
    renderMode = theRenderMode;

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

@end

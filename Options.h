//
//  Options.h
//  TrenchBroom
//
//  Created by Kristian Duske on 13.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"

typedef enum {
    RM_TEXTURED,
    RM_FLAT,
    RM_WIREFRAME
} ERenderMode;

extern NSString* const OptionsChanged;

@interface Options : Observable {
    @private
    BOOL drawGrid;
    int gridSize;
    ERenderMode renderMode;
}

- (BOOL)drawGrid;
- (int)gridSize;

- (ERenderMode)renderMode;

- (void)setDrawGrid:(BOOL)doDrawGrid;
- (void)setGridSize:(int)theGridSize;
- (void)setRenderMode:(ERenderMode)theRenderMode;

@end

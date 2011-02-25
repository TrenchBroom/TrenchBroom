//
//  Options.h
//  TrenchBroom
//
//  Created by Kristian Duske on 13.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    RM_TEXTURED,
    RM_FLAT,
    RM_WIREFRAME
} ERenderMode;

extern NSString* const OptionsChanged;

@interface Options : NSObject {
    @private
    BOOL drawGrid;
    BOOL snapToGrid;
    int gridSize;
    ERenderMode renderMode;
}

- (BOOL)drawGrid;
- (BOOL)snapToGrid;
- (int)gridSize;
- (ERenderMode)renderMode;

- (void)setDrawGrid:(BOOL)doDrawGrid;
- (void)setSnapToGrid:(BOOL)doSnapToGrid;
- (void)setGridSize:(int)theGridSize;
- (void)setRenderMode:(ERenderMode)theRenderMode;

@end

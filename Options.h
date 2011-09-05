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

typedef enum {
    IM_NONE, // no isolation
    IM_WIREFRAME, // render unselected geometry as wireframe, ignore while picking
    IM_DISCARD // do not render unselected geometry, ignore while picking
} EIsolationMode;

extern NSString* const OptionsChanged;

@class Grid;

@interface Options : NSObject {
    @private
    Grid* grid;
    ERenderMode renderMode;
    EIsolationMode isolationMode;
    BOOL renderEntities;
    BOOL renderEntityClassnames;
    BOOL renderBrushes;
    BOOL renderOrigin;
    BOOL lockTextures;
}

- (Grid *)grid;

- (ERenderMode)renderMode;
- (void)setRenderMode:(ERenderMode)theRenderMode;

- (EIsolationMode)isolationMode;
- (void)setIsolationMode:(EIsolationMode)theIsolationMode;

- (BOOL)renderEntities;
- (void)setRenderEntities:(BOOL)doRenderEntities;

- (BOOL)renderEntityClassnames;
- (void)setRenderEntityClassnames:(BOOL)doRenderEntityClassnames;

- (BOOL)renderBrushes;
- (void)setRenderBrushes:(BOOL)doRenderBrushes;

- (BOOL)renderOrigin;
- (void)setRenderOrigin:(BOOL)doRenderOrigin;

- (BOOL)lockTextures;
- (void)setLockTextures:(BOOL)doLockTextures;

@end

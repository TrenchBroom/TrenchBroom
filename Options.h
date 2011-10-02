/*
This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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

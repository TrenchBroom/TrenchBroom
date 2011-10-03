/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "Options.h"
#import "Grid.h"

NSString* const OptionsChanged = @"OptionsChanged";

@implementation Options

- (id)init {
    if ((self = [super init])) {
        grid = [[Grid alloc] init];
        renderMode = RM_TEXTURED;
        isolationMode = IM_NONE;
        renderEntities = YES;
        renderEntityClassnames = YES;
        renderBrushes = YES;
        renderOrigin = YES;
        lockTextures = YES;
    }
    
    return self;
}

- (Grid *)grid {
    return grid;
}

- (ERenderMode)renderMode {
    return renderMode;
}

- (EIsolationMode)isolationMode {
    return isolationMode;
}

- (void)setRenderMode:(ERenderMode)theRenderMode {
    if (renderMode == theRenderMode)
        return;
    
    renderMode = theRenderMode;

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (void)setIsolationMode:(EIsolationMode)theIsolationMode {
    if (isolationMode == theIsolationMode)
        return;
    
    isolationMode = theIsolationMode;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (BOOL)renderEntities {
    return renderEntities;
}

- (void)setRenderEntities:(BOOL)doRenderEntities {
    if (renderEntities == doRenderEntities)
        return;
    
    renderEntities = doRenderEntities;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (BOOL)renderEntityClassnames {
    return renderEntityClassnames;
}

- (void)setRenderEntityClassnames:(BOOL)doRenderEntityClassnames {
    if (renderEntityClassnames == doRenderEntityClassnames)
        return;
    
    renderEntityClassnames = doRenderEntityClassnames;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (BOOL)renderBrushes {
    return renderBrushes;
}

- (void)setRenderBrushes:(BOOL)doRenderBrushes {
    if (renderBrushes == doRenderBrushes)
        return;
    
    renderBrushes = doRenderBrushes;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (BOOL)renderOrigin {
    return renderOrigin;
}

- (void)setRenderOrigin:(BOOL)doRenderOrigin {
    if (renderOrigin == doRenderOrigin)
        return;
    
    renderOrigin = doRenderOrigin;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (BOOL)lockTextures {
    return lockTextures;
}

- (void)setLockTextures:(BOOL)doLockTextures {
    if (lockTextures == doLockTextures)
        return;
    
    lockTextures = doLockTextures;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center postNotificationName:OptionsChanged object:self];
}

- (void)dealloc {
    [grid release];
    [super dealloc];
}

@end

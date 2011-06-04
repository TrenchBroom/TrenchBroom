//
//  Options.m
//  TrenchBroom
//
//  Created by Kristian Duske on 13.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Options.h"
#import "Grid.h"

NSString* const OptionsChanged = @"OptionsChanged";

@implementation Options

- (id)init {
    if (self = [super init]) {
        grid = [[Grid alloc] init];
        renderMode = RM_TEXTURED;
        isolationMode = IM_NONE;
        renderEntities = YES;
        renderEntityClassnames = YES;
        renderBrushes = YES;
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


- (void)dealloc {
    [grid release];
    [super dealloc];
}

@end

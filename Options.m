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

- (void)dealloc {
    [grid release];
    [super dealloc];
}

@end

//
//  RenderBrush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <OpenGL/OpenGL.h>
#import "Brush.h"
#import "RenderBrush.h"
#import "Face.h"
#import "Polyhedron.h"
#import "Polygon3D.h"

@implementation RenderBrush

- (id)init {
    if (self = [super init]) {
        valid = NO;
    }
    
    return self;
}

- (id)initWithBrush:(Brush *)aBrush {
    if (aBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    if (self = [self init]) {
        brush = [aBrush retain];
        valid = NO;
    }
    
    return self;
}

- (Brush *)brush {
    return brush;
}

- (void)renderWithContext:(id <RenderContext>)context {
    [context renderBrush:self];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [brush release];
    [super dealloc];
}

@end

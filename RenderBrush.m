//
//  RenderBrush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <OpenGL/OpenGL.h>
#import "Brush.h"
#import "RenderContext.h"
#import "RenderBrush.h"
#import "RenderPolygon.h"
#import "Face.h"
#import "Polyhedron.h"
#import "Polygon3D.h"

@implementation RenderBrush

- (id)init {
    if (self = [super init]) {
        polygons = [[NSMutableSet alloc] init];
        polygonsValid = NO;
    }
    
    return self;
}

- (id)initWithBrush:(Brush *)aBrush {
    if (aBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    if (self = [self init]) {
        [self setBrush:aBrush];
    }
    
    return self;
}

- (void)brushChanged:(NSNotification *)notification {
    [self setBrush:[notification object]];
    [polygons removeAllObjects];
    polygonsValid = NO;
}

- (Brush *)brush {
    return brush;
}

- (void)renderWithContext:(RenderContext *)context {
    if (!polygonsValid) {
        [polygons setSet:[brush polygons]];
        polygonsValid = YES;
    }
    
    
}

- (void)setBrush:(Brush *)aBrush {
    if (aBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    [brush release];
    brush = [aBrush retain];

    [polygons removeAllObjects];
    polygonsValid = NO;
}

- (void)dealloc {
    [brush release];
    [polygons release];
    [super dealloc];
}

@end

//
//  RenderBrush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "RenderBrush.h"
#import "RenderPolygon.h"
#import "Face.h"
#import "Polyhedron.h"

@implementation RenderBrush

- (id)initWithBrush:(Brush *)aBrush {
    if (self = [super init]) {
        [self setBrush:aBrush];
    }
    
    return self;
}

- (void)brushChanged:(NSNotification *)notification {
    [self setBrush:[notification object]];
}

- (Brush *)brush {
    return brush;
}

- (void)setBrush:(Brush *)aBrush {
    if (aBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    [brush release];
    brush = [aBrush retain];
    
    [polygons release];
    polygons = [[brush polygons] retain];
}

- (void)dealloc {
    [brush release];
    [polygons release];
    [super dealloc];
}

@end

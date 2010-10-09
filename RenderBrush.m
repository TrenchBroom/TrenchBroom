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

- (id)init {
    if (self = [super init]) {
        polygons = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithBrush:(Brush *)aBrush {
    if (self = [self init]) {
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
}

- (void)dealloc {
    [brush release];
    [polygons release];

    [super dealloc];
}

@end

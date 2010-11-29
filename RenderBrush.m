//
//  RenderBrush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <OpenGL/OpenGL.h>
#import "RenderBrush.h"
#import "RenderPolygon.h"
#import "Face.h"
#import "Polyhedron.h"
#import "Polygon3D.h"

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

- (void)renderWithContext:(RenderContext *)context {
    if (polygons == nil) {
        polygons = [[NSMutableSet alloc] initWithCapacity:[brush polygons] count];
        NSEnumerator* polygonEnum = [[brush polygons] objectEnumerator];
        Polygon3D* polygon;
        while ((polygon = [polygonEnum nextObject]) != nil) {
            
        }
    }
}

- (void)setBrush:(Brush *)aBrush {
    if (aBrush == nil)
        [NSException raise:NSInvalidArgumentException format:@"brush must not be nil"];
    
    [brush release];
    brush = [aBrush retain];
    
    [polygons release];
    polygons = nil;
}

- (void)dealloc {
    [brush release];
    [polygons release];
    [super dealloc];
}

@end

//
//  MapView2D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MapView2D.h"
#include <OpenGL/gl.h>

@implementation MapView2D

-(void) drawRect:(NSRect)dirtyRect {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
}

@end

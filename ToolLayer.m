//
//  ToolLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "ToolLayer.h"
#import <OpenGL/gl.h>
#import "FaceOffsetTool.h"

@implementation ToolLayer

- (id)init {
    if (self = [super init]) {
        toolFigures = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)render:(RenderContext *)renderContext {
    NSEnumerator* figureEn = [toolFigures objectEnumerator];
    id toolFigure;
    while ((toolFigure = [figureEn nextObject])) {
        [toolFigure render];
    }
}

- (void)addFigure:(id)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    [toolFigures addObject:theFigure];
}

- (void)removeFigure:(id)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    [toolFigures removeObject:theFigure];
}

- (void)dealloc {
    [toolFigures release];
    [super dealloc];
}

@end

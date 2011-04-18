//
//  FigureLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "FigureLayer.h"
#import <OpenGL/gl.h>
#import "Figure.h"

@implementation FigureLayer

- (id)init {
    if (self = [super init]) {
        figures = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)addFigure:(id <Figure>)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    NSLog(@"adding figure %@", theFigure);
    [figures addObject:theFigure];
}

- (void)removeFigure:(id <Figure>)theFigure {
    NSAssert(theFigure != nil, @"figure must not be nil");
    NSLog(@"removing figure %@", theFigure);
    [figures removeObject:theFigure];
}

- (void)render:(RenderContext *)renderContext {
    glDisable(GL_DEPTH_TEST);
    NSEnumerator* figureEn = [figures objectEnumerator];
    id <Figure> figure;
    while ((figure = [figureEn nextObject]))
        [figure render];
    glEnable(GL_DEPTH_TEST);
}

- (void)dealloc {
    [figures release];
    [super dealloc];
}

@end

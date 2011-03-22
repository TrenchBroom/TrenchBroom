//
//  LineRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "LineRenderer.h"
#import "LineFigure.h"
#import "FigureFilter.h"
#import "VBOBuffer.h"

@interface LineRenderer (private)

- (void)validate;

@end

@implementation LineRenderer (private)

- (void)validate {
    if (!valid) {
        [vbo mapBuffer];
        vertexCount = 0;
        
        NSEnumerator* figureEn = [figures objectEnumerator];
        id <LineFigure> figure;
        while ((figure = [figureEn nextObject])) {
            if (filter == nil || [filter passes:figure]) {
                [figure updateVBO:vbo];
                vertexCount += 2;
            }
        }
        
        [vbo pack];
        [vbo unmapBuffer];
        
        valid = YES;
    }
}

@end

@implementation LineRenderer

- (id)init {
    if (self = [super init]) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        figures = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)addFigure:(id <LineFigure>)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    [figures addObject:theFigure];
    valid = NO;
}

- (void)removeFigure:(id <LineFigure>)theFigure {
    if (theFigure == nil)
        [NSException raise:NSInvalidArgumentException format:@"figure must not be nil"];
    
    [figures removeObject:theFigure];
    valid = NO;
}

- (void)setFilter:(id <FigureFilter>)theFilter {
    if (filter == theFilter)
        return;
    
    [filter release];
    filter = [theFilter retain];
    valid = NO;
}

- (void)render {
    [vbo activate];
    [self validate];
    
    glVertexPointer(3, GL_FLOAT, 0, NULL);
    glDrawArrays(GL_LINES, 0, vertexCount);
    
    [vbo deactivate];
}

- (void)dealloc {
    [figures release];
    [vbo release];
    [super dealloc];
}

@end

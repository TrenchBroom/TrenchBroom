//
//  LineRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;
@protocol LineFigure;
@protocol FigureFilter;

@interface LineRenderer : NSObject {
    NSMutableSet* figures;
    id <FigureFilter> filter;
    VBOBuffer* vbo;
    BOOL valid;
    int vertexCount;
}

- (void)addFigure:(id <LineFigure>)theFigure;
- (void)removeFigure:(id <LineFigure>)theFigure;

- (void)setFilter:(id <FigureFilter>)theFilter;
- (void)render;

@end

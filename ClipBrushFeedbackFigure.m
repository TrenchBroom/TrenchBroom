//
//  ClipBrushFeedbackFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 29.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ClipBrushFeedbackFigure.h"
#import "Brush.h"
#import "MutableBrush.h"
#import "ClipPlane.h"
#import "VertexData.h"
#import "GLUtils.h"

@implementation ClipBrushFeedbackFigure

- (id)initWithBrush:(id <Brush>)theBrush clipPlane:(ClipPlane *)theClipPlane {
    NSAssert(theBrush != nil, @"brush must not be nil");
    NSAssert(theClipPlane != nil, @"clip plane must not be nil");
    
    if (self = [self init]) {
        [theClipPlane clipBrush:theBrush firstResult:&brush1 secondResult:&brush2];
        if (brush1 == nil && brush2 == nil) {
            [self release];
            return nil;
        } else {
            if (brush1 != nil)
                [brush1 retain];
            if (brush2 != nil)
                [brush2 retain];
        }
    }
    
    return self;
}

- (void)render {
    if (brush1 != nil || brush2 != nil) {
        glColor4f(0, 1, 0, 1);
        glBegin(GL_LINES);
        if (brush1 != nil) {
            TEdge** edges = [brush1 edges];
            for (int i = 0; i < [brush1 edgeCount]; i++) {
                glVertexV3f(&edges[i]->startVertex->vector);
                glVertexV3f(&edges[i]->endVertex->vector);
            }
        }
        if (brush2 != nil) {
            TEdge** edges = [brush2 edges];
            for (int i = 0; i < [brush2 edgeCount]; i++) {
                glVertexV3f(&edges[i]->startVertex->vector);
                glVertexV3f(&edges[i]->endVertex->vector);
            }
        }
        glEnd();
    }
}

- (void)dealloc {
    [brush1 release];
    [brush2 release];
    [super dealloc];
}
@end

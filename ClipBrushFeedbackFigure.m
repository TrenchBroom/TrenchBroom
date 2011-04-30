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
#import "Edge.h"
#import "Vertex.h"
#import "ClipPlane.h"

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
            NSEnumerator* edgeEn = [[brush1 edges] objectEnumerator];
            Edge* edge;
            while ((edge = [edgeEn nextObject])) {
                TVector3f* s = [[edge startVertex] vector];
                TVector3f* e = [[edge endVertex] vector];
                glVertex3f(s->x, s->y, s->z);
                glVertex3f(e->x, e->y, e->z);
            }
        }
        if (brush2 != nil) {
            NSEnumerator* edgeEn = [[brush2 edges] objectEnumerator];
            Edge* edge;
            while ((edge = [edgeEn nextObject])) {
                TVector3f* s = [[edge startVertex] vector];
                TVector3f* e = [[edge endVertex] vector];
                glVertex3f(s->x, s->y, s->z);
                glVertex3f(e->x, e->y, e->z);
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

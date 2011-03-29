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
#import "Vector3f.h"
#import "ClipPlane.h"

@implementation ClipBrushFeedbackFigure

- (id)initWithBrush:(id <Brush>)theBrush clipPlane:(ClipPlane *)theClipPlane {
    NSAssert(theBrush != nil, @"brush must not be nil");
    NSAssert(theClipPlane != nil, @"clip plane must not be nil");
    
    if (self = [self init]) {
        brush1 = [[MutableBrush alloc] initWithTemplate:theBrush];
        MutableFace* clipFace1 = [theClipPlane clipMode] == CM_BACK ? [theClipPlane backFace] : [theClipPlane frontFace];
        if (clipFace1 != nil && ![brush1 addFace:clipFace1]) {
            [brush1 release];
            brush1 = nil;
        }
        
        if ([theClipPlane clipMode] == CM_SPLIT) {
            brush2 = [[MutableBrush alloc] initWithTemplate:theBrush];
            MutableFace* clipFace2 = [theClipPlane backFace];
            if (clipFace2 != nil && ![brush2 addFace:clipFace2]) {
                [brush2 release];
                brush2 = nil;
            }
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
                Vector3f* s = [[edge startVertex] vector];
                Vector3f* e = [[edge endVertex] vector];
                glVertex3f([s x], [s y], [s z]);
                glVertex3f([e x], [e y], [e z]);
            }
        }
        if (brush2 != nil) {
            NSEnumerator* edgeEn = [[brush2 edges] objectEnumerator];
            Edge* edge;
            while ((edge = [edgeEn nextObject])) {
                Vector3f* s = [[edge startVertex] vector];
                Vector3f* e = [[edge endVertex] vector];
                glVertex3f([s x], [s y], [s z]);
                glVertex3f([e x], [e y], [e z]);
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

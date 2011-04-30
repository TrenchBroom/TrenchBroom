//
//  ClipPointFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "Figure.h"
#import "Math.h"

@interface ClipPointFeedbackFigure : NSObject <Figure> {
    TVector3i point;
    GLUquadric* sphere;
}

- (id)initWithPoint:(TVector3i *)thePoint;

@end

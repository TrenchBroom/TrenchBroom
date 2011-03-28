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
#import "FeedbackFigure.h"

@class Vector3i;

@interface ClipPointFeedbackFigure : NSObject <FeedbackFigure> {
    Vector3i* point;
    GLUquadric* sphere;
}

- (id)initWithPoint:(Vector3i *)thePoint;

@end

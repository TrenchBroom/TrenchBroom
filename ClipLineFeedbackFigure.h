//
//  ClipLineFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "Figure.h"

@class Vector3i;

@interface ClipLineFeedbackFigure : NSObject <Figure> {
    Vector3i* startPoint;
    Vector3i* endPoint;
}

- (id)initWithStartPoint:(Vector3i *)theStartPoint endPoint:(Vector3i *)theEndPoint;

@end

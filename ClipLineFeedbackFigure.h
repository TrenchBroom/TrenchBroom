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
#import "Math.h"

@interface ClipLineFeedbackFigure : NSObject <Figure> {
    TVector3i startPoint;
    TVector3i endPoint;
}

- (id)initWithStartPoint:(TVector3i *)theStartPoint endPoint:(TVector3i *)theEndPoint;

@end

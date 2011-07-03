//
//  RotationFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Figure.h"
#import "Math.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

@interface RotationFeedbackFigure : NSObject <Figure> {
@private
    TVector3f center;
    EAxis vAxis;
    float hAngle;
    float vAngle;
    BOOL drag;
    float radius;
}

- (void)updateCenter:(TVector3f *)theCenter radius:(float)theRadius verticalAxis:(EAxis)theVerticalAxis;
- (void)setDragging:(BOOL)isDragging;
- (void)updateHorizontalAngle:(float)theHAngle verticalAngle:(float)theVAngle;

@end

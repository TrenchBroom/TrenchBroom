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
    GLUquadric* sphere;
    BOOL initialized;
    TVector3f center;
    TVector3f initialDragVector;
    TVector3f currentDragVector;
    BOOL drag;
    float radius;
}

- (void)updateCenter:(TVector3f *)theCenter radius:(float)theRadius;
- (void)setDragging:(BOOL)isDragging;
- (void)updateInitialDragVector:(TVector3f *)theVector;
- (void)updateCurrentDragVector:(TVector3f *)theVector;

@end

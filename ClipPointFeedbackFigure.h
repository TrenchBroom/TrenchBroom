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

@class Camera;

@interface ClipPointFeedbackFigure : NSObject <Figure> {
    TVector3f point;
    GLUquadric* sphere;
    Camera* camera;
}

- (id)initWithPoint:(const TVector3i *)thePoint camera:(Camera *)theCamera;

@end

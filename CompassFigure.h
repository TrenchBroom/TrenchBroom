//
//  BrushToolFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Figure.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

@class Camera;

@interface CompassFigure : NSObject <Figure> {
    GLUquadric* arms;
    GLUquadric* disks;
    Camera* camera;
}

- (id)initWithCamera:(Camera *)theCamera;

@end

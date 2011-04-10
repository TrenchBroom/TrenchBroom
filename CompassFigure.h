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

@class Vector3f;

@interface CompassFigure : NSObject <Figure> {
    GLUquadric* arms;
    GLUquadric* disks;
    Vector3f* position;
    BOOL drawX;
    BOOL drawY;
    BOOL drawZ;
}

- (void)setPosition:(Vector3f *)thePosition;
- (void)setDrawX:(BOOL)doDrawX;
- (void)setDrawY:(BOOL)doDrawY;
- (void)setDrawZ:(BOOL)doDrawZ;

@end

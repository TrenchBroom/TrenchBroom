/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Foundation/Foundation.h>
#import "Figure.h"
#import "Math.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

@interface RotateFeedbackFigure : NSObject <Figure> {
@private
    TVector3f center;
    EAxis vAxis;
    float hAngle;
    float vAngle;
    BOOL drag;
    float radius;
    float initialHAngle;
    float initialVAngle;
}

- (void)updateCenter:(TVector3f *)theCenter radius:(float)theRadius verticalAxis:(EAxis)theVerticalAxis initialHAngle:(float)theInitialHAngle initialVAngle:(float)theInitialVAngle;
- (void)setDragging:(BOOL)isDragging;
- (void)updateHorizontalAngle:(float)theHAngle verticalAngle:(float)theVAngle;

@end

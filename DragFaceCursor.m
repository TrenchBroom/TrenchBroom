/*
 Copyright (C) 2010-2011 Kristian Duske
 
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

#import "DragFaceCursor.h"
#import <OpenGL/gl.h>
#import "GLUtils.h"
#import "ArrowFigure.h"

@implementation DragFaceCursor

- (void)dealloc {
    [arrowFigure release];
    [super dealloc];
}

- (void)render {
    if (arrowFigure == nil)
        arrowFigure = [[ArrowFigure alloc] initWithDirection:&dragDirection];
    
    [arrowFigure setPosition:&position];
    [arrowFigure setCameraPosition:&cameraPosition];
    
    TVector4f fillColor1 = {0, 0, 0, 1};
    TVector4f outlineColor1 = {1, 1, 1, 1};
    TVector4f fillColor2 = {0, 0, 0, 0.3f};
    TVector4f outlineColor2 = {1, 1, 1, 0.3f};
    
    glDisable(GL_DEPTH_TEST);
    
    [arrowFigure setFillColor:&fillColor2];
    [arrowFigure setOutlineColor:&outlineColor2];
    [arrowFigure render];
    
    glEnable(GL_DEPTH_TEST);
    
    [arrowFigure setFillColor:&fillColor1];
    [arrowFigure setOutlineColor:&outlineColor1];
    [arrowFigure render];
}

- (void)setPosition:(const TVector3f *)thePosition {
    NSAssert(thePosition != NULL, @"position must not be NULL");
    position = *thePosition;
}

- (void)setDragDirection:(const TVector3f *)theDragDirection {
    NSAssert(theDragDirection != NULL, @"drag direction must not be NULL");
    dragDirection = *theDragDirection;
    
    if (arrowFigure != nil) {
        [arrowFigure release];
        arrowFigure = nil;
    }
}

- (void)setCameraPosition:(const TVector3f *)theCameraPosition {
    NSAssert(theCameraPosition != nil, @"camera position must not be nil");
    cameraPosition = *theCameraPosition;
}

@end

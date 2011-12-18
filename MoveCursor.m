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

#import "MoveCursor.h"
#import "DoubleArrowFigure.h"
#import "GLUtils.h"
#import "EditingSystem.h"
#import "ControllerUtils.h"

@implementation MoveCursor

- (id)init {
    if ((self = [super init])) {
        xArrow = [[DoubleArrowFigure alloc] initWithDirection:A_X];
        yArrow = [[DoubleArrowFigure alloc] initWithDirection:A_Y];
        zArrow = [[DoubleArrowFigure alloc] initWithDirection:A_Z];
    }
    
    return self;
}

- (void)dealloc {
    [xArrow release];
    [yArrow release];
    [zArrow release];
    [super dealloc];
}

- (void)render {
    [xArrow setPosition:&position];
    [xArrow setCameraPosition:&cameraPosition];
    [yArrow setPosition:&position];
    [yArrow setCameraPosition:&cameraPosition];
    [zArrow setPosition:&position];
    [zArrow setCameraPosition:&cameraPosition];

    TVector4f fillColor1 = {0, 0, 0, 1};
    TVector4f outlineColor1 = {1, 1, 1, 1};
    TVector4f fillColor2 = {0, 0, 0, 0.3f};
    TVector4f outlineColor2 = {1, 1, 1, 0.3f};

    glDisable(GL_DEPTH_TEST);
    
    [xArrow setFillColor:&fillColor2];
    [xArrow setOutlineColor:&outlineColor2];
    [xArrow render];
    
    [yArrow setFillColor:&fillColor2];
    [yArrow setOutlineColor:&outlineColor2];
    [yArrow render];

    [zArrow setFillColor:&fillColor2];
    [zArrow setOutlineColor:&outlineColor2];
    [zArrow render];
    
    glEnable(GL_DEPTH_TEST);

    [xArrow setFillColor:&fillColor1];
    [xArrow setOutlineColor:&outlineColor1];
    [xArrow render];
    
    [yArrow setFillColor:&fillColor1];
    [yArrow setOutlineColor:&outlineColor1];
    [yArrow render];
    
    [zArrow setFillColor:&fillColor1];
    [zArrow setOutlineColor:&outlineColor1];
    [zArrow render];
}

- (void)setEditingSystem:(EditingSystem *)theEditingSystem {
    NSAssert(theEditingSystem != nil, @"editing system must not be nil");
    editingSystem = theEditingSystem;
}

- (void)setPosition:(const TVector3f *)thePosition {
    NSAssert(thePosition != nil, @"position must not be nil");
    position = *thePosition;
}

- (void)setMoveDirection:(EMoveDirection)theMoveDirection {
    moveDirection = theMoveDirection;
}

- (void)setCameraPosition:(const TVector3f *)theCameraPosition {
    cameraPosition = *theCameraPosition;
}

@end

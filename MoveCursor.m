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
        arrows[A_X] = [[DoubleArrowFigure alloc] initWithDirection:A_X];
        arrows[A_Y] = [[DoubleArrowFigure alloc] initWithDirection:A_Y];
        arrows[A_Z] = [[DoubleArrowFigure alloc] initWithDirection:A_Z];
    }
    
    return self;
}

- (void)dealloc {
    [arrows[A_X] release];
    [arrows[A_Y] release];
    [arrows[A_Z] release];
    [super dealloc];
}

- (void)render {
    DoubleArrowFigure* arrow1;
    DoubleArrowFigure* arrow2;
    
    arrow1 = arrows[strongestComponentV3f([editingSystem xAxisPos])];
    arrow2 = arrows[strongestComponentV3f([editingSystem yAxisPos])];
    
    [arrow1 setPosition:&position];
    [arrow1 setCameraPosition:&cameraPosition];
    [arrow2 setPosition:&position];
    [arrow2 setCameraPosition:&cameraPosition];

    TVector4f fillColor1 = {0, 0, 0, 1};
    TVector4f outlineColor1 = {1, 1, 1, 1};
    TVector4f fillColor2 = {0, 0, 0, 0.3f};
    TVector4f outlineColor2 = {1, 1, 1, 0.3f};

    glDisable(GL_DEPTH_TEST);
    
    [arrow1 setFillColor:&fillColor2];
    [arrow1 setOutlineColor:&outlineColor2];
    [arrow1 render];
    
    [arrow2 setFillColor:&fillColor2];
    [arrow2 setOutlineColor:&outlineColor2];
    [arrow2 render];
    
    glEnable(GL_DEPTH_TEST);

    [arrow1 setFillColor:&fillColor1];
    [arrow1 setOutlineColor:&outlineColor1];
    [arrow1 render];
    
    [arrow2 setFillColor:&fillColor1];
    [arrow2 setOutlineColor:&outlineColor1];
    [arrow2 render];
}

- (void)setEditingSystem:(EditingSystem *)theEditingSystem {
    NSAssert(theEditingSystem != nil, @"editing system must not be nil");
    editingSystem = theEditingSystem;
}

- (void)setPosition:(const TVector3f *)thePosition {
    NSAssert(thePosition != nil, @"position must not be nil");
    position = *thePosition;
}

- (void)setCameraPosition:(const TVector3f *)theCameraPosition {
    NSAssert(theCameraPosition != nil, @"camera position must not be nil");
    cameraPosition = *theCameraPosition;
}

@end

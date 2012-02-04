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

#import "DragVertexCursor.h"
#import "DoubleArrowFigure.h"
#import "GLUtils.h"
#import "EditingSystem.h"
#import "ControllerUtils.h"

static const float shaftRadius = 0.5f;
static const float shaftLength = 4.5f;
static const float headRadius = 1.5f;
static const float headLength = 3.5f;

@implementation DragVertexCursor

- (id)init {
    if ((self = [super init])) {
        arrows[A_X] = [[DoubleArrowFigure alloc] initWithDirection:A_X shaftRadius:shaftRadius shaftLength:shaftLength headRadius:headRadius headLength:headLength];
        arrows[A_Y] = [[DoubleArrowFigure alloc] initWithDirection:A_Y shaftRadius:shaftRadius shaftLength:shaftLength headRadius:headRadius headLength:headLength];
        arrows[A_Z] = [[DoubleArrowFigure alloc] initWithDirection:A_Z shaftRadius:shaftRadius shaftLength:shaftLength headRadius:headRadius headLength:headLength];
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
    if (editingSystem == nil)
        return;
    
    DoubleArrowFigure* arrow1;
    DoubleArrowFigure* arrow2;
    
    arrow1 = arrows[strongestComponentV3f([editingSystem xAxisPos])];
    arrow2 = arrows[strongestComponentV3f([editingSystem yAxisPos])];
    
    [arrow1 setPosition:&position];
    [arrow1 setCameraPosition:&cameraPosition];
    [arrow2 setPosition:&position];
    [arrow2 setCameraPosition:&cameraPosition];

    TVector4f fillColor1, fillColor2, outlineColor1, outlineColor2;

    if (attention) {
        fillColor1 = (TVector4f) {0, 0, 0, 1};
        fillColor2 = (TVector4f) {0, 0, 0, 0.3f};
        outlineColor1 = (TVector4f) {1, 0, 0, 1};
        outlineColor2 = (TVector4f) {1, 0, 0, 0.3f};
    } else {
        fillColor1 = (TVector4f) {0, 0, 0, 1};
        fillColor2 = (TVector4f) {0, 0, 0, 0.3f};
        outlineColor1 = (TVector4f) {1, 1, 1, 1};
        outlineColor2 = (TVector4f) {1, 1, 1, 0.3f};
    }
    
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

- (void)setAttention:(BOOL)theAttention {
    attention = theAttention;
}


@end

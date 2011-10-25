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


#import "MoveToolFeedbackFigure.h"
#import "EditingSystem.h"
#import "GLUtils.h"

static const float ArrowBaseWidth = 6;
static const float ArrowBaseLength = 0.75f;
static const float ArrowHeadWidth = 10;
static const float ArrowHeadLength = 0.25f;

@interface MoveToolFeedbackFigure (private)

- (void)renderArrow:(TVector3f *)triangles color:(const TVector4f *)fillColor outline:(TVector3f *)outline outlineColor:(const TVector4f *)outlineColor;
- (void)renderArrowsWithOutlineColor:(const TVector4f *)outlineColor fillColor:(const TVector4f *)fillColor;

@end

@implementation MoveToolFeedbackFigure (private)

- (void)renderArrow:(TVector3f *)triangles color:(const TVector4f *)fillColor outline:(TVector3f *)outline outlineColor:(const TVector4f *)outlineColor {
    glColorV4f(fillColor);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 6; i++)
        glVertexV3f(&triangles[i]);
    glEnd();
    glColorV4f(outlineColor);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 6; i++)
        glVertexV3f(&outline[i]);
    glEnd();
}

- (void)renderArrowsWithOutlineColor:(const TVector4f *)outlineColor fillColor:(const TVector4f *)fillColor {
    glPushMatrix();
    glTranslatef(point.x, point.y, point.z);

    TVector3f outline[6];
    TVector3f triangles[6];
    if (moveDirection == MD_LR_FB) {
        makeArrowOutline(arrowLength * ArrowBaseLength, ArrowBaseWidth, arrowLength * ArrowHeadLength, ArrowHeadWidth, outline);
        makeArrowTriangles(arrowLength * ArrowBaseLength, ArrowBaseWidth, arrowLength * ArrowHeadLength, ArrowHeadWidth, triangles);
        
        const TMatrix4f* localToWorldMatrix = [editingSystem localToWorldMatrix];
        const TVector3f* axis = [editingSystem zAxis];
        
        for (int i = 0; i < 6; i++) {
            projectOntoCoordinatePlane(P_XY, &outline[i], &outline[i]);
            projectOntoCoordinatePlane(P_XY, &triangles[i], &triangles[i]);
            
            transformM4fV3f(localToWorldMatrix, &outline[i], &outline[i]);
            transformM4fV3f(localToWorldMatrix, &triangles[i], &triangles[i]);
        }
        
        // right arrow
        [self renderArrow:triangles color:fillColor outline:outline outlineColor:outlineColor];
        
        // left arrow
        glPushMatrix();
        glRotatef(180, axis->x, axis->y, axis->z);
        [self renderArrow:triangles color:fillColor outline:outline outlineColor:outlineColor];
        glPopMatrix();
        
        // front arrow
        glPushMatrix();
        glRotatef(90, axis->x, axis->y, axis->z);
        [self renderArrow:triangles color:fillColor outline:outline outlineColor:outlineColor];
        
        // back arrow
        glRotatef(180, axis->x, axis->y, axis->z);
        [self renderArrow:triangles color:fillColor outline:outline outlineColor:outlineColor];
        glPopMatrix();
    } else if (moveDirection == MD_LR_UD) {
        makeArrowOutline(arrowLength * ArrowBaseLength, ArrowBaseWidth, arrowLength * ArrowHeadLength, ArrowHeadWidth, outline);
        makeArrowTriangles(arrowLength * ArrowBaseLength, ArrowBaseWidth, arrowLength * ArrowHeadLength, ArrowHeadWidth, triangles);
        
        const TMatrix4f* localToWorldMatrix = [editingSystem localToWorldMatrix];
        const TVector3f* axis = [editingSystem zAxis];
        
        for (int i = 0; i < 6; i++) {
            projectOntoCoordinatePlane(P_XZ, &outline[i], &outline[i]);
            projectOntoCoordinatePlane(P_XZ, &triangles[i], &triangles[i]);
            
            transformM4fV3f(localToWorldMatrix, &outline[i], &outline[i]);
            transformM4fV3f(localToWorldMatrix, &triangles[i], &triangles[i]);
        }
        
        // right arrow
        [self renderArrow:triangles color:fillColor outline:outline outlineColor:outlineColor];
        
        // left arrow
        glPushMatrix();
        glRotatef(180, axis->x, axis->y, axis->z);
        [self renderArrow:triangles color:fillColor outline:outline outlineColor:outlineColor];
        glPopMatrix();
        
        // front arrow
        glPushMatrix();
        glRotatef(90, axis->x, axis->y, axis->z);
        [self renderArrow:triangles color:fillColor outline:outline outlineColor:outlineColor];
        
        // back arrow
        glRotatef(180, axis->x, axis->y, axis->z);
        [self renderArrow:triangles color:fillColor outline:outline outlineColor:outlineColor];
        glPopMatrix();
    }

    glPopMatrix();
}

@end

@implementation MoveToolFeedbackFigure

- (id)initWithArrowLength:(float)theArrowLength {
    NSAssert(theArrowLength > 0, @"arrow length must be positive");
    
    if ((self = [self init])) {
        arrowLength = theArrowLength;
    }
    
    return self;
}

- (void)render {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    
    TVector4f outlineColor1 = {1, 1, 0, 1};
    TVector4f fillColor1 = {1, 1, 0, 0.5f};
    TVector4f outlineColor2 = {1, 1, 0, 0.5f};
    TVector4f fillColor2 = {1, 1, 0, 0.25f};
    
    glMatrixMode(GL_MODELVIEW);
    
    glDisable(GL_DEPTH_TEST);
    [self renderArrowsWithOutlineColor:&outlineColor1 fillColor:&fillColor1];
    glEnable(GL_DEPTH_TEST);
    [self renderArrowsWithOutlineColor:&outlineColor2 fillColor:&fillColor2];
    
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_CULL_FACE);

}

- (void)setEditingSystem:(EditingSystem *)theEditingSystem {
    NSAssert(theEditingSystem != nil, @"editing system must not be nil");
    editingSystem = theEditingSystem;
}

- (void)setPoint:(const TVector3f *)thePoint {
    NSAssert(thePoint != nil, @"point must not be nil");
    point = *thePoint;
}

- (void)setMoveDirection:(EMoveDirection)theMoveDirection {
    moveDirection = theMoveDirection;
}
@end

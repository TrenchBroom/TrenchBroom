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

static const float ArrowBaseWidth = 6;
static const float ArrowBaseLength = 0.75f;
static const float ArrowHeadWidth = 10;
static const float ArrowHeadLength = 0.25f;

@interface DragFaceCursor (private)

- (void)renderArrowWithOutlineColor:(const TVector4f *)outlineColor fillColor:(const TVector4f *)fillColor;

@end

@implementation DragFaceCursor (private)

- (void)renderArrowWithOutlineColor:(const TVector4f *)outlineColor fillColor:(const TVector4f *)fillColor {
    TVector3f outline[6];
    TVector3f triangles[6];
    TVector3f planeX, planeY, planeZ;
    TMatrix4f planeMatrix;

    scaleV3f(&dragDirection, 1, &planeX);
    crossV3f(&planeX, &rayDirection, &planeY);
    normalizeV3f(&planeY, &planeY);
    crossV3f(&planeX, &planeY, &planeZ);
    
    setColumnM4fV3f(&IdentityM4f, &planeX, 0, &planeMatrix);
    setColumnM4fV3f(&planeMatrix, &planeY, 1, &planeMatrix);
    setColumnM4fV3f(&planeMatrix, &planeZ, 2, &planeMatrix);
    //invertM4f(&planeMatrix, &planeMatrix);
    
    /*
    makeArrowOutline(arrowLength * ArrowBaseLength, ArrowBaseWidth, arrowLength * ArrowHeadLength, ArrowHeadWidth, outline);
    makeArrowTriangles(arrowLength * ArrowBaseLength, ArrowBaseWidth, arrowLength * ArrowHeadLength, ArrowHeadWidth, triangles);

    for (int i = 0; i < 6; i++) {
        transformM4fV3f(&planeMatrix, &outline[i], &outline[i]);
        transformM4fV3f(&planeMatrix, &triangles[i], &triangles[i]);
    }
     */
    
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glMultMatrixf(planeMatrix.values);

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
    
    glPopMatrix();
}

@end

@implementation DragFaceCursor

- (void)render {
    /*
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    
    TVector4f outlineColor1 = {1, 1, 1, 0.25f};
    TVector4f fillColor1 = {0, 0, 0, 0.25f};
    TVector4f outlineColor2 = {1, 1, 1, 1};
    TVector4f fillColor2 = {0, 0, 0, 1};
    
    glMatrixMode(GL_MODELVIEW);
    
    glDisable(GL_DEPTH_TEST);
    [self renderArrowWithOutlineColor:&outlineColor1 fillColor:&fillColor1];
    
    glEnable(GL_DEPTH_TEST);
    glSetEdgeOffset(0.5f);
    [self renderArrowWithOutlineColor:&outlineColor2 fillColor:&fillColor2];
    glResetEdgeOffset();
    
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_CULL_FACE);
     */
}

- (void)setArrowLength:(float)theArrowLength {
    arrowLength = theArrowLength;
}

- (void)setPosition:(const TVector3f *)thePosition {
    NSAssert(thePosition != NULL, @"position must not be NULL");
    position = *thePosition;
}

- (void)setDragDirection:(const TVector3f *)theDragDirection {
    NSAssert(theDragDirection != NULL, @"drag direction must not be NULL");
    dragDirection = *theDragDirection;
}

- (void)setRayDirection:(const TVector3f *)theRayDirection {
    NSAssert(theRayDirection != NULL, @"ray direction must not be NULL");
    rayDirection = *theRayDirection;
}

@end

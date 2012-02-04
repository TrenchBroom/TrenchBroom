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

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "EntityDefinition.h"
#import "PickingHitList.h"
#import "Camera.h"
#import "Grid.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

BOOL calculateEntityOrigin(EntityDefinition* entityDefinition, PickingHitList* hits, NSPoint mousePos, Camera* camera, TVector3i* result);
NSArray* modListFromWorldspawn(id <Entity> worldspawn);
void calculateMoveDelta(Grid* grid, const TBoundingBox* bounds, const TBoundingBox* worldBounds, TVector3f* deltaf, TVector3f* lastPoint);

/**
 * Computes how far a face must be dragged along its normal so that one of its 
 * vertices snaps to the closest grid plane.
 *
 * The given drag vector is projected onto the face normal to yield the distance
 * by which the user wants to drag the face. The vertices are supposed to be 
 * snapped to the grid, so this algorithm determines the smallest drag distance
 * that snaps any of the faces vertices onto the grid along at least one
 * dimension.
 * Miniscule drags may result in the face not moving at all due to the integer
 * representation of the faces points, so this algorithm ignores such small
 * drag distances.
 *
 * @param grid the grid
 * @param face the face which is to be dragged
 * @param worldBounds the bounds of the level
 * @param deltaf the drag vector
 * @returns the distance of the face drag along its normal
 */
float calculateDragDelta(Grid* grid, id<Face> face, const TBoundingBox* worldBounds, const TVector3f* deltaf);
void updateMenuWithExecutables(NSMenu* menu, BOOL setIcons, SEL action);

void cursorArrow(const TVector3f* arrowDir, const TVector3f* viewDir, BOOL align, TVector3f** outline, int* outlineCount, TVector3f** triangles, int* triangleCount);

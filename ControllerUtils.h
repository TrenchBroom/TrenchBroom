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
float calculateDragDelta(Grid* grid, id<Face> face, const TBoundingBox* worldBounds, const TVector3f* deltaf);
void updateMenuWithExecutables(NSMenu* menu, BOOL setIcons, SEL action);

//
//  ControllerUtils.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Math.h"
#import "EntityDefinition.h"
#import "PickingHitList.h"
#import "Camera.h"
#import "Grid.h"
#import "Entity.h"

BOOL calculateEntityOrigin(EntityDefinition* entityDefinition, PickingHitList* hits, NSPoint mousePos, Camera* camera, TVector3i* result);
NSArray* modListFromWorldspawn(id <Entity> worldspawn);
void calculateMoveDelta(Grid* grid, const TBoundingBox* bounds, const TBoundingBox* worldBounds, TVector3f* deltaf, TVector3f* lastPoint);
void updateMenuWithExecutables(NSMenu* menu, BOOL setIcons, SEL action);

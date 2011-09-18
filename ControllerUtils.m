//
//  ControllerUtils.m
//  TrenchBroom
//
//  Created by Kristian Duske on 28.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ControllerUtils.h"
#import "PreferencesManager.h"
#import "Face.h"

BOOL calculateEntityOrigin(EntityDefinition* entityDefinition, PickingHitList* hits, NSPoint mousePos, Camera* camera, TVector3i* result) {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        const TVector3f* hitPoint = [hit hitPoint];
        const TBoundingBox* bounds = [entityDefinition bounds];
        TVector3f size;
        sizeOfBounds(bounds, &size);
        
        id <Face> face = [hit object];
        const TVector3f* faceNorm = [face norm];
        
        EAxis lc = strongestComponentV3f(faceNorm);
        BOOL d;
        switch (lc) {
            case A_X:
                if (faceNorm->x > 0) {
                    result->x = hitPoint->x - bounds->min.x;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = YES;
                } else {
                    result->x = hitPoint->x - size.x - bounds->min.x;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = NO;
                }
                break;
            case A_Y:
                if (faceNorm->y > 0) {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = YES;
                } else {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - size.y - bounds->min.y;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = NO;
                }
                break;
            case A_Z:
                if (faceNorm->z > 0) {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z;
                    d = YES;
                } else {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - size.z - bounds->min.z;
                    d = NO;
                }
                break;
        }
        
        return YES;
    } else {
        TVector3f posf = [camera defaultPointAtX:mousePos.x y:mousePos.y];
        roundV3f(&posf, result);
        return NO;
    }
}

NSArray* modListFromWorldspawn(id <Entity> worldspawn) {
    if (![worldspawn isWorldspawn])
        return nil;
    
    NSString* modsStr = [worldspawn propertyForKey:ModsKey];
    NSMutableArray* mods = [[NSMutableArray alloc] init];
    [mods addObject:@"id1"];
    if (modsStr != nil) {
        NSEnumerator* modStrEn = [[modsStr componentsSeparatedByString:@";"] objectEnumerator];
        NSString* mod;
        while ((mod = [modStrEn nextObject])) {
            NSString* cleaned = [mod stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
            if ([cleaned length] > 0)
                [mods addObject:cleaned];
        }
    }
    
    return [mods autorelease];
}

void calculateMoveDelta(Grid* grid, const TBoundingBox* bounds, const TBoundingBox* worldBounds, TVector3f* deltaf, TVector3f* lastPoint, TVector3f* point) {
    if (deltaf->x > 0) {
        deltaf->x = [grid snapDownToGridf:bounds->max.x + deltaf->x] - bounds->max.x;
        if (deltaf->x <= 0) {
            deltaf->x = 0;
        } else {
            if (deltaf->x < 1)
                deltaf->x = [grid actualSize];
            if (bounds->max.x + deltaf->x > worldBounds->max.x) {
                deltaf->x = worldBounds->max.x - bounds->max.x;
                deltaf->y = 0;
                deltaf->z = 0;
            } else if (lastPoint != NULL && point != NULL) {
                lastPoint->x = point->x;
            }
        }
    } else if (deltaf->x < 0) {
        deltaf->x = [grid snapUpToGridf:bounds->min.x + deltaf->x] - bounds->min.x;
        if (deltaf->x >= 0) {
            deltaf->x = 0;
        } else {
            if (deltaf->x > -1)
                deltaf->x = -[grid actualSize];
            if (bounds->min.x + deltaf->x < worldBounds->min.x) {
                deltaf->x = worldBounds->min.x - bounds->min.x;
                deltaf->y = 0;
                deltaf->z = 0;
            } else if (lastPoint != NULL && point != NULL) {
                lastPoint->x = point->x;
            }
        }
    }
    
    if (deltaf->y > 0) {
        deltaf->y = [grid snapDownToGridf:bounds->max.y + deltaf->y] - bounds->max.y;
        if (deltaf->y <= 0) {
            deltaf->y = 0;
        } else {
            if (deltaf->y < 1)
                deltaf->y = [grid actualSize];
            if (bounds->max.y + deltaf->y > worldBounds->max.y) {
                deltaf->x = 0;
                deltaf->y = worldBounds->max.y - bounds->max.y;
                deltaf->z = 0;
            } else if (lastPoint != NULL && point != NULL) {
                lastPoint->y = point->y;
            }
        }
    } else if (deltaf->y < 0) {
        deltaf->y = [grid snapUpToGridf:bounds->min.y + deltaf->y] - bounds->min.y;
        if (deltaf->y >= 0) {
            deltaf->y = 0;
        } else {
            if (deltaf->y > -1)
                deltaf->y = -[grid actualSize];
            if (bounds->min.y + deltaf->y < worldBounds->min.y) {
                deltaf->x = 0;
                deltaf->y = worldBounds->min.y - bounds->min.y;
                deltaf->z = 0;
            } else if (lastPoint != NULL && point != NULL) {
                lastPoint->y = point->y;
            }
        }
    }
    
    if (deltaf->z > 0) {
        deltaf->z = [grid snapDownToGridf:bounds->max.z + deltaf->z] - bounds->max.z;
        if (deltaf->z <= 0) {
            deltaf->z = 0;
        } else {
            if (deltaf->z < 1)
                deltaf->z = [grid actualSize];
            if (bounds->max.z + deltaf->z > worldBounds->max.z) {
                deltaf->x = 0;
                deltaf->y = 0;
                deltaf->z = worldBounds->max.z - bounds->max.z;
            } else if (lastPoint != NULL && point != NULL) {
                lastPoint->z = point->z;
            }
        }
    } else if (deltaf->z < 0) {
        deltaf->z = [grid snapUpToGridf:bounds->min.z + deltaf->z] - bounds->min.z;
        if (deltaf->z >= 0) {
            deltaf->z = 0;
        } else {
            if (deltaf->z > -1)
                deltaf->z = -[grid actualSize];
            if (bounds->min.z + deltaf->z < worldBounds->min.z) {
                deltaf->x = 0;
                deltaf->y = 0;
                deltaf->z = worldBounds->min.z < bounds->min.z;
            } else if (lastPoint != NULL && point != NULL) {
                lastPoint->z = point->z;
            }
        }
    }
}

void updateMenuWithExecutables(NSMenu* menu, BOOL setIcons, SEL action) {
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSArray* executables = [preferences availableExecutables];

    NSEnumerator* executableEn = [executables objectEnumerator];
    NSString* executable;
    int index = 0;
    
    while ((executable = [executableEn nextObject])) {
        NSString* filename = [executable lastPathComponent];
        NSString* appname = [filename stringByDeletingPathExtension];
        NSMenuItem* menuItem = [[NSMenuItem alloc] initWithTitle:appname action:action keyEquivalent:@""];
        if (setIcons)
            [menuItem setImage:[workspace iconForFile:executable]];
        [menuItem setTag:index++];
        [menu addItem:menuItem];
        [menuItem release];
    }
}

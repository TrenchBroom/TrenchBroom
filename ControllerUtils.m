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

#import "ControllerUtils.h"
#import "PreferencesManager.h"
#import "MutableFace.h"

static const float ArrowBaseWidth = 7;
static const float ArrowBaseLength = 11;
static const float ArrowHeadWidth = 10;
static const float ArrowHeadLength = 5;

BOOL calculateEntityOrigin(Grid* grid, EntityDefinition* entityDefinition, PickingHitList* hits, NSPoint mousePos, Camera* camera, TVector3i* result) {
    const TBoundingBox* bounds = [entityDefinition bounds];
    TVector3f size;
    sizeOfBounds(bounds, &size);

    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        const TVector3f* hitPoint = [hit hitPoint];
        
        id <Face> face = [hit object];
        const TVector3f* faceNorm = [face norm];
        
        EAxis lc = strongestComponentV3f(faceNorm);
        BOOL d;
        switch (lc) {
            case A_X:
                if (faceNorm->x > 0) {
                    result->x = hitPoint->x - bounds->min.x;
                    result->y = [grid snapToGridf:hitPoint->y - bounds->min.y - size.y / 2];
                    result->z = [grid snapToGridf:hitPoint->z - bounds->min.z - size.z / 2];
                    d = YES;
                } else {
                    result->x = hitPoint->x - size.x - bounds->min.x;
                    result->y = [grid snapToGridf:hitPoint->y - bounds->min.y - size.y / 2];
                    result->z = [grid snapToGridf:hitPoint->z - bounds->min.z - size.z / 2];
                    d = NO;
                }
                break;
            case A_Y:
                if (faceNorm->y > 0) {
                    result->x = [grid snapToGridf:hitPoint->x - bounds->min.x - size.x / 2];
                    result->y = hitPoint->y - bounds->min.y;
                    result->z = [grid snapToGridf:hitPoint->z - bounds->min.z - size.z / 2];
                    d = YES;
                } else {
                    result->x = [grid snapToGridf:hitPoint->x - bounds->min.x - size.x / 2];
                    result->y = hitPoint->y - size.y - bounds->min.y;
                    result->z = [grid snapToGridf:hitPoint->z - bounds->min.z - size.z / 2];
                    d = NO;
                }
                break;
            case A_Z:
                if (faceNorm->z > 0) {
                    result->x = [grid snapToGridf:hitPoint->x - bounds->min.x - size.x / 2];
                    result->y = [grid snapToGridf:hitPoint->y - bounds->min.y - size.y / 2];
                    result->z = hitPoint->z - bounds->min.z;
                    d = YES;
                } else {
                    result->x = [grid snapToGridf:hitPoint->x - bounds->min.x - size.x / 2];
                    result->y = [grid snapToGridf:hitPoint->y - bounds->min.y - size.y / 2];
                    result->z = hitPoint->z - size.z - bounds->min.z;
                    d = NO;
                }
                break;
        }
        
        return YES;
    } else {
        TVector3f posf = [camera defaultPointAtX:mousePos.x y:mousePos.y];
        scaleV3f(&size, 0.5f, &size);
        subV3f(&posf, &size, &posf);
        [grid snapToGridV3f:&posf result:&posf];
        roundV3f(&posf, &posf);
        setV3i(result, &posf);
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

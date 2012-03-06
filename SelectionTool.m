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

#import "SelectionTool.h"
#import "MapWindowController.h"
#import "PickingHitList.h"
#import "PickingHit.h"
#import "SelectionManager.h"
#import "Face.h"
#import "Brush.h"
#import "Entity.h"
#import "Options.h"
#import "Grid.h"

@interface SelectionTool (private)

- (BOOL)isMultiSelectionModifierPressed;
- (BOOL)isGridSizeModifierPressed;

@end

@implementation SelectionTool (private)

- (BOOL)isMultiSelectionModifierPressed {
    return ([NSEvent modifierFlags] & NSCommandKeyMask) == NSCommandKeyMask;
}

- (BOOL)isGridSizeModifierPressed {
    return [NSEvent modifierFlags] == NSAlternateKeyMask;
}

@end


@implementation SelectionTool

- (BOOL)beginLeftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    return [self isMultiSelectionModifierPressed];
}

- (void)leftDrag:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    SelectionManager* selectionManager = [windowController selectionManager];
    PickingHit* hit = [hits firstHitOfType:HT_ENTITY | HT_FACE ignoreOccluders:YES];
    
    if (hit != nil) {
        if ([hit type] == HT_ENTITY) {
            id <Entity> entity= [hit object];
            if (![entity selected]) {
                    [selectionManager addEntity:[hit object] record:NO];
            }
        } else {
            id <Face> face = [hit object];
            id <Brush> brush = [face brush];
            if ([selectionManager mode] == SM_FACES) {
                if (![face selected])
                    [selectionManager addFace:face record:NO];
            } else if (![brush selected]) {
                [selectionManager addBrush:brush record:NO];
            }
        }
    }
}

- (BOOL)scrollWheel:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    if (![self isGridSizeModifierPressed])
        return NO;
    
    Grid* grid = [[windowController options] grid];
    int size = [grid size];
    if ([event deltaY] < 0)
        size -= 1;
    else
        size += 1;
    
    if (size >= GridMinSize && size <= GridMaxSize)
        [grid setSize:size];
    
    return YES;
}

- (BOOL)leftMouseUp:(NSEvent *)event ray:(TRay *)ray hits:(PickingHitList *)hits {
    SelectionManager* selectionManager = [windowController selectionManager];
    PickingHit* hit = [hits firstHitOfType:HT_ENTITY | HT_FACE ignoreOccluders:YES];

    if (hit != nil) {
        if ([hit type] == HT_ENTITY) {
            id <Entity> entity = [hit object];
            
            if ([entity selected]) {
                if ([self isMultiSelectionModifierPressed]) {
                    [selectionManager removeEntity:entity record:NO];
                } else {
                    [selectionManager removeAll:NO];
                    [selectionManager addEntity:entity record:NO];
                }
            } else {
                if (![self isMultiSelectionModifierPressed])
                    [selectionManager removeAll:NO];
                [selectionManager addEntity:entity record:NO];
                
            }
        } else {
            id <Face> face = [hit object];
            id <Brush> brush = [face brush];
            
            if ([selectionManager mode] == SM_FACES) {
                if ([face selected]) {
                    if ([self isMultiSelectionModifierPressed])
                        [selectionManager removeFace:face record:NO];
                    else
                        [selectionManager addBrush:brush record:NO];
                } else {
                    if ([self isMultiSelectionModifierPressed]) {
                        [selectionManager addFace:face record:NO];
                    } else if ([NSEvent modifierFlags] == 0) {
                        if ([selectionManager isBrushPartiallySelected:brush]) {
                            [selectionManager removeAll:NO];
                            [selectionManager addFace:face record:NO];
                        } else {
                            [selectionManager addBrush:brush record:NO];
                        }
                    }
                }
            } else {
                if ([self isMultiSelectionModifierPressed]) {
                    if ([brush selected]) {
                        [selectionManager removeBrush:brush record:NO];
                    } else {
                        [selectionManager addBrush:brush record:NO];
                    }
                } else if ([NSEvent modifierFlags] == 0) {
                    if ([brush selected]) {
                        [selectionManager addFace:face record:NO];
                    } else {
                        [selectionManager removeAll:NO];
                        [selectionManager addBrush:brush record:NO];
                    }
                }
            }
        }
    } else {
        [selectionManager removeAll:NO];
    }
    
    return YES;
}

@end

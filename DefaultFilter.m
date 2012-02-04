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

#import "DefaultFilter.h"
#import "SelectionManager.h"
#import "GroupManager.h"
#import "Options.h"
#import "Entity.h"
#import "Brush.h"
#import "Face.h"

@implementation DefaultFilter

- (id)initWithSelectionManager:(SelectionManager *)theSelectionManager groupManager:(GroupManager *)theGroupManager options:(Options *)theOptions {
    NSAssert(theSelectionManager != nil, @"selection manager must not be nil");
    NSAssert(theGroupManager != nil, @"group manager must not be nil");
    NSAssert(theOptions != nil, @"options must not be nil");
    
    if ((self = [self init])) {
        groupManager = theGroupManager;
        selectionManager = theSelectionManager;
        options = theOptions;
    }
    
    return self;
}

- (BOOL)isBrushRenderable:(id<Brush>)brush {
    if (![options renderBrushes])
        return NO;
    
    if ([options isolationMode] == IM_DISCARD) {
        if ([selectionManager mode] == SM_FACES) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject]))
                if ([selectionManager isFaceSelected:face])
                    return YES;
            
            return NO;
        }
        
        if ([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_BRUSHES_ENTITIES) {
            return [selectionManager isBrushSelected:brush];
        }
        
        return NO;
    }
    
    if ([groupManager allGroupsInvisible])
        return YES;
    
    id <Entity> entity = [brush entity];
    if ([GroupClassName isEqualToString:[entity classname]])
        return [groupManager isVisible:entity];
    
    return NO;
}

- (BOOL)isEntityRenderable:(id<Entity>)entity {
    if ([entity isWorldspawn])
        return YES;
    
    if (![options renderEntities])
        return NO;
    
    if ([options isolationMode] == IM_DISCARD)
        return [selectionManager isEntitySelected:entity];

    if ([groupManager allGroupsInvisible])
        return YES;
    
    if ([GroupClassName isEqualToString:[entity classname]])
        return [groupManager isVisible:entity];
    
    return YES;
}

- (BOOL)isBrushPickable:(id<Brush>)brush {
    if (![options renderBrushes])
        return NO;
    
    if ([options isolationMode] != IM_NONE) {
        if ([selectionManager mode] == SM_FACES) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject]))
                if ([selectionManager isFaceSelected:face])
                    return YES;
            
            return NO;
        }

        if ([selectionManager mode] == SM_BRUSHES || [selectionManager mode] == SM_BRUSHES_ENTITIES)
            return [selectionManager isBrushSelected:brush];
            
        return NO;
    }
    
    if ([groupManager allGroupsInvisible])
        return YES;
    
    id <Entity> entity = [brush entity];
    if ([GroupClassName isEqualToString:[entity classname]])
        return [groupManager isVisible:entity];
    
    return NO;
}

- (BOOL)isEntityPickable:(id<Entity>)entity {
    if (![options renderEntities])
        return NO;
    
    if ([options isolationMode] == IM_NONE)
        return YES;
    
    return [selectionManager isEntitySelected:entity];
}

@end

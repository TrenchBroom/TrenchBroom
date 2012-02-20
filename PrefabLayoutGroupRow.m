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

#import "PrefabLayoutGroupRow.h"
#import "PrefabLayoutPrefabCell.h"
#import "PrefabGroup.h"
#import "Prefab.h"
#import "GLFontManager.h"
#import "GLString.h"

@implementation PrefabLayoutGroupRow

- (id)init {
    if ((self = [super init])) {
        cells = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithPrefabGroup:(id <PrefabGroup>)thePrefabGroup prefabsPerRow:(int)thePrefabsPerRow atPos:(NSPoint)thePos width:(float)theWidth innerMargin:(float)innerMargin fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if ((self = [self init])) {
        prefabGroup = [thePrefabGroup retain];
        name = [[theFontManager createGLString:[prefabGroup name] font:theFont] retain];
        NSSize nameSize = [name size];
        titleBarBounds = NSMakeRect(thePos.x, thePos.y, theWidth, nameSize.height + 4);
        titleBounds = NSMakeRect(thePos.x + 4, thePos.y + 2, nameSize.width, nameSize.height);

        float x = thePos.x;
        float y = NSMaxY(titleBarBounds) + innerMargin;
        float cellWidth = theWidth / thePrefabsPerRow - (thePrefabsPerRow - 1) * innerMargin;
        PrefabLayoutPrefabCell* cell;
        
        NSArray* prefabs = [prefabGroup prefabs];
        for (int i = 0; i < [prefabs count]; i++) {
            if (i > 0 && i % thePrefabsPerRow == 0) {
                x = thePos.x;
                y += cellWidth + innerMargin;
            }

            id <Prefab> prefab = [prefabs objectAtIndex:i];
            GLString* prefabName = [theFontManager createGLString:[prefab name] font:theFont];
            cell = [[PrefabLayoutPrefabCell alloc] initWithPrefab:prefab atPos:NSMakePoint(x, y) width:cellWidth name:prefabName];
            [cells addObject:cell];
            [cell release];
            
            x += cellWidth + innerMargin;
        }

        float height = NSMaxY([cell bounds]) - thePos.y;
        
        bounds = NSMakeRect(thePos.x, thePos.y, theWidth, height);
    }
    
    return self;
}

- (id <PrefabGroup>)prefabGroup {
    return prefabGroup;
}

- (GLString *)name {
    return name;
}

- (NSArray *)cells {
    return cells;
}

- (id <Prefab>)prefabAt:(NSPoint)pos {
    for (PrefabLayoutPrefabCell* cell in cells)
        if (NSPointInRect(pos, [cell prefabBounds]))
            return [cell prefab];
    return nil;
}

- (NSRect)titleBarBounds {
    return titleBarBounds;
}

- (NSRect)titleBounds {
    return titleBounds;
}

- (NSRect)bounds {
    return bounds;
}

- (void)dealloc {
    [prefabGroup release];
    [name release];
    [cells release];
    [super dealloc];
}

@end

/*
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

#import "PrefabLayout.h"
#import "PrefabManager.h"
#import "PrefabGroup.h"
#import "PrefabLayoutGroupRow.h"
#import "GLFontManager.h"

@implementation PrefabLayout

- (id)init {
    if ((self = [super init])) {
        groupRows = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithPrefabManager:(PrefabManager *)thePrefabManager prefabsPerRow:(int)thePrefabsPerRow fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if ((self = [self init])) {
        prefabManager = [thePrefabManager retain];
        prefabsPerRow = thePrefabsPerRow;
        fontManager = [theFontManager retain];
        font = [theFont retain];
        outerMargin = 10;
        innerMargin = 5;
        groupMargin = 10;
    }
    
    return self;
}

- (void)validate {
    [groupRows removeAllObjects];
    
    NSEnumerator* groupEn = [[prefabManager prefabGroups] objectEnumerator];
    id <PrefabGroup> group;
    int y = outerMargin;
    
    while ((group = [groupEn nextObject])) {
        PrefabLayoutGroupRow* groupRow = [[PrefabLayoutGroupRow alloc] initWithPrefabGroup:group prefabsPerRow:prefabsPerRow atPos:NSMakePoint(outerMargin, y) width:width - 2 * outerMargin innerMargin:innerMargin fontManager:fontManager font:font];
        [groupRows addObject:groupRow];
        y += NSHeight([groupRow bounds]) + groupMargin;
        [groupRow release];
    }
    
    height = y - groupMargin + outerMargin;
    valid = YES;
}

- (NSArray *)groupRows {
    if (!valid)
        [self validate];
    
    return groupRows;
}

- (float)height {
    if (!valid)
        [self validate];
    
    return height;
}

- (id <Prefab>)prefabAt:(NSPoint)pos {
    if (!valid)
        [self validate];
    
    NSEnumerator* groupRowEn = [groupRows objectEnumerator];
    PrefabLayoutGroupRow* groupRow;
    while ((groupRow = [groupRowEn nextObject]))
        if (NSPointInRect(pos, [groupRow bounds]))
            return [groupRow prefabAt:(NSPoint)pos];
    
    return nil;
}

- (void)setPrefabsPerRow:(int)thePrefabsPerRow {
    prefabsPerRow = thePrefabsPerRow;
    [self invalidate];
}

- (void)setWidth:(float)theWidth {
    width = theWidth;
    [self invalidate];
}

- (void)invalidate {
    valid = NO;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [groupRows release];
    [prefabManager release];
    [fontManager release];
    [font release];
    [super dealloc];
}
@end

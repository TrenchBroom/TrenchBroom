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

#import "EntityDefinitionLayout.h"
#import "EntityDefinition.h"
#import "EntityDefinitionLayoutCell.h"
#import "GLFontManager.h"
#import "GLString.h"
#import "EntityDefinitionFilter.h"

@implementation EntityDefinitionLayout

- (id)init {
    if ((self = [super init])) {
        rows = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithFontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if ((self = [self init])) {
        fontManager = [theFontManager retain];
        font = [theFont retain];
        outerMargin = 10;
        innerMargin = 5;
    }
    
    return self;
}

- (void)validate {
    [rows removeAllObjects];
    
    if (entityDefinitions == nil || [entityDefinitions count] == 0)
        return;
    
    float x = width; // to force creation of first row
    float y = outerMargin;
    float yd = 0;
    int i = 0;
    
    float maxWidth = 150;

    NSMutableArray* row;
    for (EntityDefinition* definition in entityDefinitions) {
        if (filter == nil || [filter passes:definition]) {
            GLString* cellName = [fontManager createGLString:[definition name] font:font];
            float cellWidth = fmax(maxWidth, [cellName size].width);
            
            if (x + cellWidth + outerMargin > width) {
                row = [[NSMutableArray alloc] init];
                [rows addObject:row];
                [row release];
                x = outerMargin;
                y += yd + innerMargin;
                yd = 0;
            }
            
            EntityDefinitionLayoutCell* cell = [[EntityDefinitionLayoutCell alloc] initWithEntityDefinition:definition atPos:NSMakePoint(x, y) width:cellWidth name:cellName];
            [row addObject:cell];
            [cell release];
            
            NSRect cellBounds = [cell bounds];
            x += cellBounds.size.width + innerMargin;
            if (cellBounds.size.height > yd)
                yd = cellBounds.size.height;
            i++;
        }
    }

    height = y + yd + outerMargin;
    valid = YES;
}

- (NSArray *)rows {
    if (!valid)
        [self validate];
    
    return rows;
}

- (float)height {
    if (!valid)
        [self validate];
    
    return height;
}

- (EntityDefinitionLayoutCell *)cellAt:(NSPoint)pos {
    if (!valid)
        [self validate];
    
    for (NSArray* row in rows)
        for (EntityDefinitionLayoutCell* cell in row)
            if (NSPointInRect(pos, [cell bounds]))
                return cell;
    
    return nil;
}

- (EntityDefinition *)entityDefinitionAt:(NSPoint)pos {
    EntityDefinitionLayoutCell* cell = [self cellAt:pos];
    if (cell == nil)
        return nil;
    
    return [cell entityDefinition];
}

- (void)setEntityDefinitions:(NSArray *)theEntityDefinitions {
    [entityDefinitions release];
    entityDefinitions = [theEntityDefinitions retain];
    [self invalidate];
}

- (void)setEntityDefinitionFilter:(id <EntityDefinitionFilter>)theFilter {
    [filter release];
    filter = [theFilter retain];
    [self invalidate];
}

- (void)setWidth:(float)theWidth {
    if (width == theWidth)
        return;
    width = theWidth;
    [self invalidate];
}

- (void)invalidate {
    valid = NO;
}

- (void)clear {
    [self invalidate];
}

- (void)dealloc {
    [rows release];
    [entityDefinitions release];
    [fontManager release];
    [font release];
    [filter release];
    [super dealloc];
}
@end

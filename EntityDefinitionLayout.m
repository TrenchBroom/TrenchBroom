//
//  EntityDefinitionLayout.m
//  TrenchBroom
//
//  Created by Kristian Duske on 15.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityDefinitionLayout.h"
#import "EntityDefinitionManager.h"
#import "EntityDefinition.h"
#import "EntityDefinitionLayoutCell.h"
#import "GLFontManager.h"
#import "GLString.h"

@implementation EntityDefinitionLayout

- (id)init {
    if ((self = [super init])) {
        rows = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithEntityDefinitionManager:(EntityDefinitionManager *)theEntityDefinitionManager fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if ((self = [self init])) {
        entityDefinitionManager = [theEntityDefinitionManager retain];
        fontManager = [theFontManager retain];
        font = [theFont retain];
        outerMargin = 10;
        innerMargin = 5;
    }
    
    return self;
}

- (void)validate {
    [rows removeAllObjects];
    
    NSEnumerator* definitionEn = [[entityDefinitionManager definitionsOfType:EDT_POINT] objectEnumerator];
    EntityDefinition* definition;
    float x = width; // to force creation of first row
    float y = outerMargin;
    float yd = 0;
    int i = 0;
    
    float maxWidth = 200;

    NSMutableArray* row;
    while ((definition = [definitionEn nextObject])) {
        GLString* nameString = [fontManager glStringFor:[definition name] font:font];
        float cellWidth = fmax(maxWidth, [nameString size].width);
        
        if (x + cellWidth + outerMargin > width) {
            row = [[NSMutableArray alloc] init];
            [rows addObject:row];
            [row release];
            x = outerMargin;
            y += yd + innerMargin;
            yd = 0;
        }

        EntityDefinitionLayoutCell* cell = [[EntityDefinitionLayoutCell alloc] initWithEntityDefinition:definition atPos:NSMakePoint(x, y) width:cellWidth nameString:nameString];
        [row addObject:cell];
        [cell release];
        
        NSRect cellBounds = [cell bounds];
        x += cellBounds.size.width + innerMargin;
        if (cellBounds.size.height > yd)
            yd = cellBounds.size.height;
        i++;
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
    
    NSEnumerator* rowEn = [rows objectEnumerator];
    NSArray* row;
    while ((row = [rowEn nextObject])) {
        NSEnumerator* cellEn = [row objectEnumerator];
        EntityDefinitionLayoutCell* cell;
        while ((cell = [cellEn nextObject])) {
            if (NSPointInRect(pos, [cell bounds]))
                return cell;
        }
    }
    
    return nil;
}

- (EntityDefinition *)entityDefinitionAt:(NSPoint)pos {
    EntityDefinitionLayoutCell* cell = [self cellAt:pos];
    if (cell == nil)
        return nil;
    
    return [cell entityDefinition];
}

- (void)setWidth:(float)theWidth {
    width = theWidth;
    [self invalidate];
}

- (void)invalidate {
    valid = NO;
}

- (void)dealloc {
    [rows release];
    [entityDefinitionManager release];
    [fontManager release];
    [font release];
    [super dealloc];
}
@end

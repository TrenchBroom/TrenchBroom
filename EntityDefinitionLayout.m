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

- (id)initWithEntityDefinitionManager:(EntityDefinitionManager *)theEntityDefinitionManager entityDefinitionsPerRow:(int)theEntityDefinitionsPerRow fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if ((self = [self init])) {
        entityDefinitionManager = [theEntityDefinitionManager retain];
        entityDefinitionsPerRow = theEntityDefinitionsPerRow;
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
    int x;
    int y = outerMargin;
    int i = 0;
    int yd;
    float cellWidth = (width - 2 * outerMargin) / entityDefinitionsPerRow - (entityDefinitionsPerRow - 1) * innerMargin;

    NSMutableArray* row;
    while ((definition = [definitionEn nextObject])) {
        if (i % entityDefinitionsPerRow == 0) {
            row = [[NSMutableArray alloc] initWithCapacity:entityDefinitionsPerRow];
            [rows addObject:row];
            [row release];
            x = outerMargin;
            y += yd;
            yd = 0;
        }
        
        GLString* nameString = [fontManager glStringFor:[definition name] font:font];
        EntityDefinitionLayoutCell* cell = [[EntityDefinitionLayoutCell alloc] initWithEntityDefinition:definition atPos:NSMakePoint(x, y) width:cellWidth nameSize:[nameString size]];
        [row addObject:cell];
        [cell release];
    }

    height = y + outerMargin;
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

- (EntityDefinition *)entityDefinitionAt:(NSPoint)pos {
    if (!valid)
        [self validate];
    
    NSEnumerator* rowEn = [rows objectEnumerator];
    NSArray* row;
    while ((row = [rowEn nextObject])) {
        NSEnumerator* cellEn = [row objectEnumerator];
        EntityDefinitionLayoutCell* cell;
        while ((cell = [cellEn nextObject])) {
            if (NSPointInRect(pos, [cell bounds]))
                return [cell entityDefinition];
        }
    }
    
    return nil;
}

- (void)setEntityDefinitionsPerRow:(int)theEntityDefinitionsPerRow {
    entityDefinitionsPerRow = theEntityDefinitionsPerRow;
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
    [rows release];
    [entityDefinitionManager release];
    [fontManager release];
    [font release];
    [super dealloc];
}
@end

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

#import "TextureViewLayout.h"
#import "TextureViewLayoutRow.h"
#import "TextureViewLayoutCell.h"
#import "GLFontManager.h"
#import "GLString.h"

@implementation TextureViewLayout

- (id)init {
    if (self = [super init]) {
        rows = [[NSMutableArray alloc] init];
        textures = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithWidth:(float)theWidth innerMargin:(float)theInnerMargin outerMargin:(float)theOuterMargin fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont {
    if (self = [self init]) {
        width = theWidth;
        innerMargin = theInnerMargin;
        outerMargin = theOuterMargin;
        fontManager = [theFontManager retain];
        font = [theFont retain];
    }
    
    return self;
}

- (void)addTexture:(Texture *)theTexture {
    [textures addObject:theTexture];
    valid = NO;
}

- (void)addTextures:(NSArray *)theTextures {
    [textures addObjectsFromArray:theTextures];
    valid = NO;
}

- (void)clear {
    [textures removeAllObjects];
    valid = NO;
}

- (void)setWidth:(float)theWidth {
    if (width == theWidth)
        return;
    
    width = theWidth;
    valid = NO;
}

- (void)layout {
    if (!valid) {
        [rows removeAllObjects];
        
        for (Texture* texture in textures) {
            if (filter == nil || [filter passes:texture]) {
                GLString* nameString = [fontManager glStringFor:[texture name] font:font];
                NSSize nameSize = [nameString size];
                TextureViewLayoutRow* row = [rows lastObject];
                if (row == nil || ![row addTexture:texture nameSize:nameSize]) {
                    float y = row == nil ? outerMargin : [row y] + [row height] + innerMargin;
                    row = [[TextureViewLayoutRow alloc] initAtY:y width:width innerMargin:innerMargin outerMargin:outerMargin];
                    [row addTexture:texture nameSize:nameSize];
                    [rows addObject:row];
                    [row release];
                }
            }
        }
        valid = YES;
    }
}

- (float)height {
    float height = 2 * outerMargin;
    if ([rows count] > 0)
        height += ([rows count] - 1) * innerMargin;

    NSEnumerator* rowEn = [rows objectEnumerator];
    TextureViewLayoutRow* row;
    while ((row = [rowEn nextObject])) {
        height += [row height];
    }
    
    return height;
}

- (NSArray *)rows {
    return rows;
}

- (NSArray *)rowsInY:(float)y height:(float)height {
    NSMutableArray* result = [[NSMutableArray alloc] init];
    for (TextureViewLayoutRow* row in rows)
        if ([row y] + [row height] > y && [row y] < y + height)
            [result addObject:row];
    
    return [result autorelease];
}

- (Texture *)textureAt:(NSPoint)location {
    for (TextureViewLayoutRow* row in rows)
        if ([row containsY:location.y])
            return [[row cellAt:location] texture];
    
    return nil;
}

- (void)setTextureFilter:(id <TextureFilter>)theFilter {
    [filter release];
    filter = [theFilter retain];
    valid = NO;
}

- (void)dealloc {
    [filter release];
    [textures release];
    [rows release];
    [fontManager release];
    [font release];
    [super dealloc];
}

@end

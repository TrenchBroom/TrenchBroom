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

#import "TextureViewLayoutRow.h"
#import "Texture.h"
#import "TextureViewLayoutCell.h"

@implementation TextureViewLayoutRow

- (id)initAtY:(float)yPos width:(float)theWidth innerMargin:(float)theInnerMargin outerMargin:(float)theOuterMargin {
    if (self = [self init]) {
        cells = [[NSMutableArray alloc] init];
        y = yPos;
        width = theWidth;
        innerMargin = theInnerMargin;
        outerMargin = theOuterMargin;
        height = 0;
    }
    
    return self;
}

- (BOOL)addTexture:(Texture *)texture nameSize:(NSSize)nameSize {
    float x;
    if ([cells count] == 0)
        x = outerMargin;
    else
        x = [[cells lastObject] cellRect].origin.x + [[cells lastObject] cellRect].size.width + innerMargin;
    
    if (x + fmax([texture width], nameSize.width) + outerMargin > width)
        return NO;
    
    TextureViewLayoutCell* cell = [[TextureViewLayoutCell alloc] initAt:NSMakePoint(x, y) texture:texture nameSize:nameSize];
    [cells addObject:cell];
    [cell release];

    height = fmax(height, [cell cellRect].size.height);
    return YES;
}

- (NSArray *)cells {
    return cells;
}

- (float)y {
    return y;
}

- (float)height {
    return height;
}

- (BOOL)containsY:(float)yCoord {
    return yCoord >= y && yCoord <= y + height;
}

- (TextureViewLayoutCell *)cellAt:(NSPoint)location {
    if (![self containsY:location.y])
        return nil;
    
    NSEnumerator* cellEn = [cells objectEnumerator];
    TextureViewLayoutCell* cell;
    while ((cell = [cellEn nextObject])) {
        if ([cell contains:location])
            return cell;
    }
    
    return nil;
}

- (void)dealloc {
    [cells release];
    [super dealloc];
}

@end

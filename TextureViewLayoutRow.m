//
//  TextureViewRow.m
//  TrenchBroom
//
//  Created by Kristian Duske on 22.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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
        x = [[cells lastObject] cellRect].origin.x + [[cells lastObject] textureWidth] + innerMargin;
    
    if (x + [texture width] + outerMargin > width)
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

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

- (BOOL)addTexture:(Texture *)texture {
    float x;
    if ([cells count] == 0)
        x = outerMargin;
    else
        x = [[cells lastObject] x] + [[cells lastObject] textureWidth] + innerMargin;
    
    if (x + [texture width] + outerMargin > width)
        return NO;
    
    height = fmax(height, [texture height]);
    
    TextureViewLayoutCell* cell = [[TextureViewLayoutCell alloc] initAtX:x texture:texture];
    [cells addObject:cell];
    [cell release];
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


- (void)dealloc {
    [cells release];
    [super dealloc];
}

@end

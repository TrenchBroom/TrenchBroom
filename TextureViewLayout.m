//
//  TextureViewManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 22.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TextureViewLayout.h"
#import "TextureViewLayoutRow.h"
#import "TextureViewLayoutCell.h"
#import "GLFont.h"
#import "math.h"

@implementation TextureViewLayout

- (id)init {
    if (self = [super init]) {
        rows = [[NSMutableArray alloc] init];
        textures = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithWidth:(float)theWidth innerMargin:(float)theInnerMargin outerMargin:(float)theOuterMargin font:(GLFont *)theFont {
    if (theFont == nil)
        [NSException raise:NSInvalidArgumentException format:@"font must not be nil"];
    
    if (self = [self init]) {
        width = theWidth;
        innerMargin = theInnerMargin;
        outerMargin = theOuterMargin;
        font = [theFont retain];
    }
    
    return self;
}

- (void)addTexture:(Texture *)theTexture {
    if (theTexture == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture must not be nil"];
    
    [textures addObject:theTexture];
    [self layout];
}

- (void)addTextures:(NSArray *)theTextures {
    if (textures == nil)
        [NSException raise:NSInvalidArgumentException format:@"textures must not be nil"];
    
    [textures addObjectsFromArray:theTextures];
    [self layout];
}

- (void)clear {
    [textures removeAllObjects];
    [self layout];
}

- (void)setWidth:(float)theWidth {
    if (width == theWidth)
        return;
    
    width = theWidth;
    [self layout];
}

- (void)layout {
    [rows removeAllObjects];
    
    NSEnumerator* texEn = [textures objectEnumerator];
    Texture* texture;
    while ((texture = [texEn nextObject])) {
        if (filter == nil || [filter passes:texture]) {
            NSSize nameSize = [font sizeOfString:[texture name]];
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
    
    NSEnumerator* rowEn = [rows objectEnumerator];
    TextureViewLayoutRow* row;
    while ((row = [rowEn nextObject]))
        if ([row y] + [row height] > y && [row y] < y + height)
            [result addObject:row];
    
    return [result autorelease];
}

- (Texture *)textureAt:(NSPoint)location {
    NSEnumerator* rowEn = [rows objectEnumerator];
    TextureViewLayoutRow* row;
    while ((row = [rowEn nextObject]))
        if ([row containsY:location.y])
            return [[row cellAt:location] texture];
    
    return nil;
}

- (void)setTextureFilter:(id <TextureFilter>)theFilter {
    [filter release];
    filter = [theFilter retain];
    [self layout];
}

- (void)dealloc {
    [filter release];
    [textures release];
    [rows release];
    [font release];
    [super dealloc];
}

@end

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
        nameSizeCapacity = 64;
        nameSizes = malloc(64 * sizeof(NSSize));
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

- (void)resizeNameSizesToMinCapacity:(int)newMinCapacity {
    
    int newCapacity = nameSizeCapacity;
    while (newCapacity < newMinCapacity)
        newCapacity *= 2;
    NSSize* newSizes = malloc(newCapacity * sizeof(NSSize));
    memcpy(newSizes, nameSizes, nameSizeCapacity * sizeof(NSSize));
    free(nameSizes);
    nameSizes = newSizes;
    nameSizeCapacity = newCapacity;
}

- (void)addTexture:(Texture *)theTexture {
    if (theTexture == nil)
        [NSException raise:NSInvalidArgumentException format:@"texture must not be nil"];
    
    if ([textures count] >= nameSizeCapacity)
        [self resizeNameSizesToMinCapacity:[textures count]];
    
    [textures addObject:theTexture];
    nameSizes[[textures count]] = [font sizeOfString:[theTexture name]];

    [self layout];
}

- (void)addTextures:(NSArray *)theTextures {
    if (textures == nil)
        [NSException raise:NSInvalidArgumentException format:@"textures must not be nil"];

    if ([textures count] + [theTextures count] >= nameSizeCapacity)
        [self resizeNameSizesToMinCapacity:[textures count] + [theTextures count]];

    NSEnumerator* textureEn = [theTextures objectEnumerator];
    Texture* texture;
    while ((texture = [textureEn nextObject])) {
        nameSizes[[textures count]] = [font sizeOfString:[texture name]];
        [textures addObject:texture];
    }

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
    int i = 0;
    while ((texture = [texEn nextObject])) {
        if (filter == nil || [filter passes:texture]) {
            TextureViewLayoutRow* row = [rows lastObject];
            if (row == nil || ![row addTexture:texture nameSize:nameSizes[i]]) {
                float y = row == nil ? outerMargin : [row y] + [row height] + innerMargin;
                row = [[TextureViewLayoutRow alloc] initAtY:y width:width innerMargin:innerMargin outerMargin:outerMargin];
                [row addTexture:texture nameSize:nameSizes[i]];
                [rows addObject:row];
                [row release];
            }
        }
        i++;
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
    free(nameSizes);
    [rows release];
    [font release];
    [super dealloc];
}

@end

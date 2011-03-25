//
//  Wad.m
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Wad.h"

@implementation Wad

- (id)init {
    if (self = [super init]) {
        paletteEntries = [[NSMutableArray alloc] init];
        textureEntries = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)aName {
    if (self = [self init]) {
        name = [aName retain];
    }
    
    return self;
}

- (void)addPaletteEntry:(WadPaletteEntry *)entry {
    [paletteEntries addObject:entry];
}

- (void)addTextureEntry:(WadTextureEntry *)entry {
    [textureEntries addObject:entry];
}

- (NSArray *)paletteEntries {
    return paletteEntries;
}

- (NSArray *)textureEntries {
    return textureEntries;
}

- (NSString *)name {
    return name;
}

- (void)dealloc {
    [name release];
    [paletteEntries release];
    [textureEntries release];
    [super dealloc];
}

@end

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
    if (aName == nil)
        [NSException raise:NSInvalidArgumentException format:@"name must not be nil"];
    
    if (self = [self init]) {
        name = [aName retain];
    }
    
    return self;
}

- (void)addPaletteEntry:(WadPaletteEntry *)entry {
    if (entry == nil)
        [NSException raise:NSInvalidArgumentException format:@"entry must not be nil"];
    
    [paletteEntries addObject:entry];
}

- (void)addTextureEntry:(WadTextureEntry *)entry {
    if (entry == nil)
        [NSException raise:NSInvalidArgumentException format:@"entry must not be nil"];
    
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
    [paletteEntries release];
    [textureEntries release];
    [super dealloc];
}

@end

//
//  Wad.h
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WadPaletteEntry;
@class WadTextureEntry;

@interface Wad : NSObject {
    @private
    NSMutableArray* paletteEntries;
    NSMutableArray* textureEntries;
    NSString* name;
}

- (id)initWithName:(NSString *)aName;

- (void)addPaletteEntry:(WadPaletteEntry *)entry;
- (void)addTextureEntry:(WadTextureEntry *)entry;

- (NSArray *)paletteEntries;
- (NSArray *)textureEntries;

- (NSString *)name;

@end

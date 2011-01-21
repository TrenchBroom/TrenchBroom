//
//  TextureManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const UnknownTextureNameException;
extern NSString* const MissingPaletteException;

@class Wad;

@interface TextureManager : NSObject {
    @private
    NSMutableDictionary* textures;
    NSData* palette;
}

- (id)initWithPalette:(NSData *)thePalette;

- (void)loadTexturesFrom:(Wad *)wad;

- (void)activateTexture:(NSString *)name;
- (void)deactivateTexture;

- (void)disposeTextures;

@end

//
//  TextureManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    SC_NAME,
    SC_USAGE
} ESortCriterion;

extern NSString* const TexturesRemoved;
extern NSString* const TexturesAdded;

extern NSString* const UserInfoTextures;

extern NSString* const UnknownTextureNameException;
extern NSString* const MissingPaletteException;

@class Texture;
@class Wad;

@interface TextureManager : NSObject {
    @private
    NSMutableDictionary* textures;
    NSData* palette;
}

- (id)initWithPalette:(NSData *)thePalette;

- (void)loadTexturesFrom:(Wad *)wad;
- (void)removeAllTextures;

- (Texture *)textureForName:(NSString *)name;
- (NSArray *)texturesForNames:(NSArray *)names;
- (NSArray *)textures:(ESortCriterion)sortCriterion;

- (void)activateTexture:(NSString *)name;
- (void)deactivateTexture;

- (void)incUsageCount:(NSString *)name;
- (void)decUsageCount:(NSString *)name;

@end

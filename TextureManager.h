//
//  TextureManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const TextureManagerChanged;

extern NSString* const UnknownTextureNameException;
extern NSString* const MissingPaletteException;

@class Texture;
@class TextureCollection;

@interface TextureManager : NSObject {
    @private
    NSMutableArray* textureCollections;
    NSMutableDictionary* textures;
    NSMutableDictionary* usageCounts;
    NSMutableArray* texturesByName;
    BOOL valid;
}

- (void)addTextureCollection:(TextureCollection *)theCollection;
- (void)removeTextureCollection:(NSString *)theName;
- (void)removeAllTextureCollections;
- (NSArray *)textureCollections;

- (Texture *)textureForName:(NSString *)name;
- (NSArray *)texturesByName;

- (void)activateTexture:(NSString *)name;
- (void)deactivateTexture;

- (NSString *)wadProperty;

@end

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

extern NSString* const TextureManagerChanged;

extern NSString* const UnknownTextureNameException;
extern NSString* const MissingPaletteException;

@class Texture;
@class TextureCollection;

@interface TextureManager : NSObject {
    @private
    NSMutableArray* textureCollections;
    NSMutableDictionary* textures;
    NSMutableArray* texturesByName;
    NSMutableArray* texturesByUsageCount;
    BOOL valid;
}

- (void)addTextureCollection:(TextureCollection *)theCollection;
- (void)removeTextureCollection:(NSString *)theName;
- (void)removeAllTextureCollections;

- (Texture *)textureForName:(NSString *)name;
- (NSArray *)textures:(ESortCriterion)sortCriterion;

- (void)activateTexture:(NSString *)name;
- (void)deactivateTexture;

- (void)incUsageCount:(NSString *)name;
- (void)decUsageCount:(NSString *)name;

@end

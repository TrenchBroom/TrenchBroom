//
//  TextureManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    TS_NAME,
    TS_USAGE
} ETextureSortCriterion;

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
    BOOL valid;
}

- (void)addTextureCollection:(TextureCollection *)theCollection atIndex:(NSUInteger)theIndex;
- (void)removeTextureCollectionAtIndex:(NSUInteger)theIndex;
- (NSArray *)textureCollections;
- (void)clear;

- (void)resetUsageCounts;

- (Texture *)textureForName:(NSString *)name;
- (NSArray *)texturesByCriterion:(ETextureSortCriterion)criterion;

- (void)activateTexture:(NSString *)name;
- (void)deactivateTexture;

- (NSString *)wadProperty;

@end

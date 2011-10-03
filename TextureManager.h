/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

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

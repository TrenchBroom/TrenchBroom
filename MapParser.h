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
#import "Math.h"

typedef enum {
    PS_DEF, // default state
    PS_ENT, // currently parsing an entity
    PS_BRUSH, // currently parsing a brush
} EParserState;

typedef enum {
    CC_UNDEFINED,
    CC_ENT,
    CC_BRUSH,
    CC_FACE
} EClipboardContents;

typedef enum {
    MF_STANDARD,
    MF_VALVE,
    MF_UNDEFINED
} EMapFormat;

@protocol Map;
@class EntityDefinitionManager;
@class MapTokenizer;
@class MutableEntity;
@class MutableBrush;
@class TextureManager;

@interface MapParser : NSObject {
    @private
    int size;
    MapTokenizer* tokenizer;
    id<Map> map;
    NSMutableArray* tokens;
    EMapFormat format;
}

- (id)initWithData:(NSData *)someData;
- (id)initWithData:(NSData *)someData;
- (void)parseMap:(id<Map>)theMap textureManager:(TextureManager *)theTextureManager withProgressIndicator:(NSProgressIndicator *)theIndicator;
- (EClipboardContents)parseClipboard:(NSMutableArray *)result worldBounds:(const TBoundingBox *)theWorldBounds textureManager:(TextureManager *)theTextureManager;

@end

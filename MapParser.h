//
//  MapParser.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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
- (void)parseMap:(id<Map>)theMap withProgressIndicator:(NSProgressIndicator *)theIndicator;
- (EClipboardContents)parseClipboard:(NSMutableArray *)result worldBounds:(TBoundingBox *)theWorldBounds;

@end

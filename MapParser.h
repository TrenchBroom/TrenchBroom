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
    MutableEntity* entity;
    MutableBrush* brush;
    EParserState state;
    TVector3i p1, p2, p3;
}

- (id)initWithData:(NSData *)someData;
- (id)initWithData:(NSData *)someData;
- (void)parseMap:(id<Map>)theMap withProgressIndicator:(NSProgressIndicator *)theIndicator;

@end

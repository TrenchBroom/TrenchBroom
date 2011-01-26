//
//  MapParser.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const InvalidTokenException;

typedef enum {
    PS_DEF, // default state
    PS_ENT, // currently parsing an entity
    PS_BRUSH, // currently parsing a brush
} EParserState;


@class MapTokenizer;
@class Map;
@class Entity;
@class Brush;
@class Vector3i;

@interface MapParser : NSObject {
    @private
    MapTokenizer* tokenizer;
    Map* map;
    Entity* entity;
    Brush* brush;
    EParserState state;
    Vector3i* p1;
    Vector3i* p2;
    Vector3i* p3;
}

- (id)initWithData:(NSData *)someData;
- (Map *)parse;

@end

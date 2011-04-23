//
//  MapParser.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    PS_DEF, // default state
    PS_ENT, // currently parsing an entity
    PS_BRUSH, // currently parsing a brush
} EParserState;

@protocol Map;
@class MapTokenizer;
@class MutableEntity;
@class MutableBrush;
@class Vector3i;

@interface MapParser : NSObject {
    @private
    int size;
    MapTokenizer* tokenizer;
    id<Map> map;
    MutableEntity* entity;
    MutableBrush* brush;
    EParserState state;
    Vector3i* p1;
    Vector3i* p2;
    Vector3i* p3;
}

- (id)initWithData:(NSData *)someData;
- (void)parseMap:(id<Map>)theMap withProgressIndicator:(NSProgressIndicator *)theIndicator;

@end

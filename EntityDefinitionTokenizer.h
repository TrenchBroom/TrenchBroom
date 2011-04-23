//
//  EntityDefinitionTokenizer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    TS_OUTDEF, // currently between definitions
    TS_INDEF, // currently parsing a definition
    TS_COM, // comment
    TS_DEC, // current token is a decimal number
    TS_FRAC, // current token is a fractional number
    TS_WORD, // current token is a word
    TS_Q_STR, // current token is a quoted string
    TS_EOF // parsing is complete
} ETokenizerState;

@class EntityDefinitionToken;

@interface EntityDefinitionTokenizer : NSObject {
    NSString* definitionString;
    ETokenizerState state;
    int index;
    int line;
    int column;
    int c;
    EntityDefinitionToken* token;
}

- (id)initWithDefinitionString:(NSString *)theDefinitionString;

- (EntityDefinitionToken *)nextToken;
- (EntityDefinitionToken *)peekToken;
- (NSString *)remainderAsDescription;


@end

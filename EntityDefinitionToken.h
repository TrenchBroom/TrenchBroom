//
//  EntityDefinitionToken.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    TT_FRAC = 1 <<  0, // fractional number
    TT_DEC  = 1 <<  1, // decimal number
    TT_STR  = 1 <<  2, // string
    TT_B_O  = 1 <<  3, // opening brace
    TT_B_C  = 1 <<  4, // closing brace
    TT_CB_O = 1 <<  5, // opening curly brace
    TT_CB_C = 1 <<  6, // closing curly brace
    TT_WORD = 1 <<  7, // word
    TT_QM   = 1 <<  8, // question mark
    TT_ED_O = 1 <<  9, // entity definition open
    TT_ED_C = 1 << 10, // entity definition close
    TT_SC   = 1 << 11, // semicolon
    TT_NL   = 1 << 12, // newline
    TT_C    = 1 << 13  // comma
    
} ETokenType;

@interface EntityDefinitionToken : NSObject {
    ETokenType type;
    id data;
    int line;
    int column;
    int charsRead;
}

+ (NSString *)typeName:(int)aType;

- (ETokenType)type;
- (id)data;

- (int)line;
- (int)column;

- (int)charsRead;

- (EntityDefinitionToken *)setType:(ETokenType)theType data:(id)theData line:(int)theLine column:(int)theColumn charsRead:(int)theCharsRead;

@end

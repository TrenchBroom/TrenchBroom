//
//  Token.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    TT_FRAC = 1 << 0, // fractional number
    TT_DEC  = 1 << 1, // decimal number
    TT_STR  = 1 << 2, // string
    TT_B_O  = 1 << 3, // opening brace
    TT_B_C  = 1 << 4, // closing brace
    TT_CB_O = 1 << 5, // opening curly brace
    TT_CB_C = 1 << 6, // closing curly brace
    TT_COM  = 1 << 7 // comment
} ETokenType;


@interface MapToken : NSObject {
    @private
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

- (MapToken *)setType:(ETokenType)theType data:(id)theData line:(int)theLine column:(int)theColumn charsRead:(int)theCharsRead;

@end

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
    TT_B_O  = 1 << 3, // opening parenthesis
    TT_B_C  = 1 << 4, // closing parenthesis
    TT_CB_O = 1 << 5, // opening curly bracket
    TT_CB_C = 1 << 6, // closing curly bracket
    TT_SB_O = 1 << 7, // opening square bracket
    TT_SB_C = 1 << 8, // closing square bracket
    TT_COM  = 1 << 9 // comment
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

- (id)initWithToken:(MapToken *)theToken;

- (ETokenType)type;
- (id)data;

- (int)line;
- (int)column;

- (int)charsRead;

- (MapToken *)setType:(ETokenType)theType data:(id)theData line:(int)theLine column:(int)theColumn charsRead:(int)theCharsRead;

@end

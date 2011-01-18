//
//  Tokenizer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    TS_DEF, // default state
    TS_DEC, // current token is a decimal number
    TS_FRAC, // current token is a fractional number
    TS_STR, // current token is a string
    TS_Q_STR, // current token is a quoted string
    TS_EOF // parsing is complete
} ETokenizerState;


@class Token;

@interface Tokenizer : NSObject {
    @private
    NSInputStream* stream;
    ETokenizerState state;
    uint8_t buffer[1024];
    NSMutableString* string;
    int bufferSize;
    int bufferIndex;
    int line;
    int column;
}

- (id)initWithInputStream:(NSInputStream *)aStream;

- (Token *)nextToken;

@end

//
//  Tokenizer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Tokenizer.h"
#import "Token.h"

@implementation Tokenizer

- (id)initWithInputStream:(NSInputStream *)aStream {
    if (aStream == nil)
        [NSException raise:NSInvalidArgumentException format:@"stream must not be nil"];
    
    if (self = [self init]) {
        stream = [aStream retain];
        [stream open];
        bufferIndex = 0;
        bufferSize = [stream read:buffer maxLength:1024];
        line = 1;
        column = 0;
        state = TS_DEF;
    }
    
    return self;
}

- (char)readChar {
    if (state == EOF)
        [NSException raise:@"Invalid State" format:@"read past end of file"];
    
    if (bufferIndex == bufferSize) {
        bufferIndex = 0;
        bufferSize = [stream read:buffer maxLength:1024];
        if (bufferSize <= 0) {
            state = TS_EOF;
            return 0;
        }
    }
    
    char c = (char)buffer[bufferIndex++];
    if (c == '\n') {
        line++;
        column = 0;
    } else {
        column++;
    }
    
    return c;
}

- (Token *)nextToken {
    char c;
    while ((c = [self readChar]) != 0) {
        switch (state) {
            case TS_DEF:
                switch (c) {
                    case '\n':
                    case '\t':
                    case ' ':
                        break; // ignore whitespace in boundaries
                    case '{':
                        return [Token tokenWithType:TT_CB_O data:nil line:line column:column];
                    case '}':
                        return [Token tokenWithType:TT_CB_C data:nil line:line column:column];
                    case '(':
                        return [Token tokenWithType:TT_B_O data:nil line:line column:column];
                    case ')':
                        return [Token tokenWithType:TT_B_C data:nil line:line column:column];
                    case '"':
                        state = TS_Q_STR;
                        string = [[NSMutableString alloc] init];
                        break;
                    case '-':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        state = TS_DEC;
                        string = [[NSMutableString alloc] init];
                        [string appendFormat:@"%c", c];
                        break;
                    default:
                        state = TS_STR;
                        string = [[NSMutableString alloc] init];
                        [string appendFormat:@"%c", c];
                        break;
                }
                break;
            case TS_Q_STR:
                switch (c) {
                    case '"': {
                        Token* token = [Token tokenWithType:TT_STR data:string line:line column:column];
                        [string release];
                        string = nil;
                        state = TS_DEF;
                        return token;
                    default:
                        [string appendFormat:@"%c", c];
                        break;
                    }
                }
                break;
            case TS_STR:
                switch (c) {
                    case '\n':
                    case '\t':
                    case ' ': {
                        Token* token = [Token tokenWithType:TT_STR data:string line:line column:column];
                        [string release];
                        string = nil;
                        state = TS_DEF;
                        return token;
                    default:
                        [string appendFormat:@"%c", c];
                        break;
                    }
                }
                break;
            case TS_DEC:
                if (c == '.')
                    state = TS_FRAC;
            case TS_FRAC:
                switch (c) {
                    case '\n':
                    case '\t':
                    case ' ': {
                        NSNumber* number;
                        if (state == TS_DEC) {
                            number = [NSNumber numberWithInt:[string intValue]];
                            [string release];
                            string = nil;
                            state = TS_DEF;
                            return [Token tokenWithType:TT_DEC data:number line:line column:column];
                        } else {
                            number = [NSNumber numberWithFloat:[string floatValue]];
                            [string release];
                            string = nil;
                            state = TS_DEF;
                            return [Token tokenWithType:TT_FRAC data:number line:line column:column];
                        }
                        break;
                    }
                    default:
                        [string appendFormat:@"%c", c];
                        break;
                }
            default:
                break;
        }
    }
    
    return nil;
}

- (void)dealloc {
    [stream release];
    [string release];
    [super dealloc];
}

@end

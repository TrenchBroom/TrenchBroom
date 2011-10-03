/*
Copyright (C) 2010-2011 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Cocoa/Cocoa.h>

typedef enum {
    TS_DEF, // default state
    TS_DEC, // current token is a decimal number
    TS_FRAC, // current token is a fractional number
    TS_STR, // current token is a string
    TS_Q_STR, // current token is a quoted string
    TS_COM,
    TS_EOF // parsing is complete
} ETokenizerState;


@class MapToken;

@interface MapTokenizer : NSObject {
    @private
    NSInputStream* stream;
    ETokenizerState state;
    MapToken* token;
    uint8_t buffer[1024];
    NSMutableString* string;
    int bufferSize;
    int bufferIndex;
    NSMutableString* pushBuffer;
    int line;
    int column;
    int startLine;
    int startColumn;
    int charsRead;
}

- (id)initWithInputStream:(NSInputStream *)aStream;

- (MapToken *)nextToken;

@end

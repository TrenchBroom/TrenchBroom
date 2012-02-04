/*
Copyright (C) 2010-2012 Kristian Duske

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

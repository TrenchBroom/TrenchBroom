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

#import "EntityDefinitionToken.h"


@implementation EntityDefinitionToken
+ (NSString *)typeName:(int)aType {
    NSMutableString* result = [[NSMutableString alloc] init];
    if ((aType & TT_FRAC) != 0)
        [result appendString:@"fraction, "];
    
    if ((aType & TT_DEC) != 0)
        [result appendString:@"decimal, "];
    
    if ((aType & TT_STR) != 0)
        [result appendString:@"string, "];
    
    if ((aType & TT_B_O) != 0)
        [result appendString:@"opening brace, "];
    
    if ((aType & TT_B_C) != 0)
        [result appendString:@"closing brace, "];
    
    if ((aType & TT_CB_O) != 0)
        [result appendString:@"opening curly brace, "];
    
    if ((aType & TT_CB_C) != 0)
        [result appendString:@"closing curly brace, "];
    
    if ((aType & TT_WORD) != 0)
        [result appendString:@"word, "];
        
    if ((aType & TT_QM) != 0)
        [result appendString:@"question mark, "];
    
    if ((aType & TT_ED_O) != 0)
        [result appendString:@"entity definition open, "];
    
    if ((aType & TT_ED_C) != 0)
        [result appendString:@"entity definition close, "];
    
    if ((aType & TT_SC) != 0)
        [result appendString:@"semicolon, "];
    
    if ((aType & TT_NL) != 0)
        [result appendString:@"newline, "];
    
    if ((aType & TT_C) != 0)
        [result appendString:@"comma, "];
    
    if ([result length] > 0)
        [result deleteCharactersInRange:NSMakeRange([result length] - 2, 2)];
    
    return [result autorelease];
}

- (ETokenType)type {
    return type;
}

- (id)data {
    return data;
}

- (int)line {
    return line;
}

- (int)column {
    return column;
}

- (int)charsRead {
    return charsRead;
}

- (EntityDefinitionToken *)setType:(ETokenType)theType data:(id)theData line:(int)theLine column:(int)theColumn charsRead:(int)theCharsRead {
    type = theType;
    [data release];
    data = [theData retain];
    line = theLine;
    column = theColumn;
    charsRead = theCharsRead;
    
    return self;
}

- (NSString *)description {
    switch (type) {
        case TT_FRAC:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@, data: %f", line, column, [EntityDefinitionToken typeName:type], [data floatValue]];
        case TT_DEC:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@, data: %i", line, column, [EntityDefinitionToken typeName:type], [data intValue]];
        case TT_WORD:
        case TT_ED_O:
        case TT_ED_C:
        case TT_STR:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@, data: %@", line, column, [EntityDefinitionToken typeName:type], data];
        case TT_B_O:
        case TT_B_C:
        case TT_CB_O:
        case TT_CB_C:
        case TT_QM:
        case TT_SC:
        case TT_NL:
        case TT_C:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@", line, column, [EntityDefinitionToken typeName:type]];
        default:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@, data: %@", line, column, data, [EntityDefinitionToken typeName:type]];
    }
}

- (void)dealloc {
    [data release];
    [super dealloc];
}
@end

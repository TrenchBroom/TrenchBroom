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

#import "MapToken.h"


@implementation MapToken

+ (NSString *)typeName:(int)aType {
    NSMutableString* result = [[NSMutableString alloc] init];
    if ((aType & TT_FRAC) != 0)
        [result appendString:@"fraction, "];
    
    if ((aType & TT_DEC) != 0)
        [result appendString:@"decimal, "];
    
    if ((aType & TT_STR) != 0)
        [result appendString:@"string, "];
    
    if ((aType & TT_B_O) != 0)
        [result appendString:@"opening parenthesis, "];
    
    if ((aType & TT_B_C) != 0)
        [result appendString:@"closing parenthesis, "];
    
    if ((aType & TT_CB_O) != 0)
        [result appendString:@"opening curly bracket, "];
    
    if ((aType & TT_CB_C) != 0)
        [result appendString:@"closing curly bracket, "];
    
    if ((aType & TT_SB_O) != 0)
        [result appendString:@"opening square bracket, "];
    
    if ((aType & TT_SB_C) != 0)
        [result appendString:@"closing square bracket, "];
    
    if ([result length] > 0)
        [result deleteCharactersInRange:NSMakeRange([result length] - 2, 2)];
    
    return [result autorelease];
}

- (id)initWithToken:(MapToken *)theToken {
    NSAssert(theToken != nil, @"token must not be nil");
    
    if ((self = [self init])) {
        [self setType:[theToken type] data:[theToken data] line:[theToken line] column:[theToken column] charsRead:[theToken charsRead]];
    }
    
    return self;
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

- (MapToken *)setType:(ETokenType)theType data:(id)theData line:(int)theLine column:(int)theColumn charsRead:(int)theCharsRead {
    [data release];
    
    type = theType;
    data = [theData retain];
    line = theLine;
    column = theColumn;
    charsRead = theCharsRead;
    
    return self;
}

- (NSString *)description {
    switch (type) {
        case TT_FRAC:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@, data: %f", line, column, [MapToken typeName:type], [data floatValue]];
        case TT_DEC:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@, data: %i", line, column, [MapToken typeName:type], [data intValue]];
        case TT_STR:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@, data: %@", line, column, [MapToken typeName:type], data];
        case TT_B_O:
        case TT_B_C:
        case TT_CB_O:
        case TT_CB_C:
        case TT_SB_O:
        case TT_SB_C:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@", line, column, [MapToken typeName:type]];
        default:
            return [NSString stringWithFormat:@"position: %i,%i, type: %@, data: %@", line, column, data, [MapToken typeName:type]];
    }
}

- (void)dealloc {
    [data release];
    [super dealloc];
}

@end

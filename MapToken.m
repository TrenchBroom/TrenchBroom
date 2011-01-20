//
//  Token.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MapToken.h"


@implementation MapToken
+ (MapToken *)tokenWithType:(ETokenType)aType data:(id)someData line:(int)aLine column:(int)aColumn {
    return [[[MapToken alloc] initWithType:aType data:someData line:aLine column:aColumn] autorelease];
}

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
    
    if ([result length] > 0)
        [result deleteCharactersInRange:NSMakeRange([result length] - 2, 2)];
    
    return result;
}

- (id)initWithType:(ETokenType)aType data:(id)someData line:(int)aLine column:(int)aColumn {
    if (self = [self init]) {
        type = aType;
        data = [someData retain];
        line = aLine;
        column = aColumn;
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

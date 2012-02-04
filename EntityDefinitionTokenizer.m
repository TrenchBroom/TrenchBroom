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

#import "EntityDefinitionTokenizer.h"
#import "EntityDefinitionToken.h"

@interface EntityDefinitionTokenizer (private)

- (BOOL)next;
- (char)peek:(int)offset;
- (char)peek;

@end

@implementation EntityDefinitionTokenizer (private)

- (BOOL)next {
    if (state == TS_EOF)
        [NSException raise:@"Invalid State" format:@"read past end of file"];
    
    if (index + 1 == [definitionString length]) {
        state = TS_EOF;
        return NO;
    }
    
    c = [definitionString characterAtIndex:++index];
    if (c == '\n') {
        line++;
        column = 0;
    } else {
        column++;
    }
    
    return c;
}

- (void)pushBack {
    if (index <= 0)
        [NSException raise:@"Invalid State" format:@"read past start of file"];

    if (state == TS_EOF)
        state = TS_OUTDEF;
    
    c = [definitionString characterAtIndex:--index];
    if (c == '\n') {
        line--;
        column = 0;
        int i = index - 1;
        while (i >= 0 && [definitionString characterAtIndex:i] != '\n') {
            i--;
            column++;
        }
    } else {
        column--;
    }
}

- (char)peek:(int)offset {
    if (index + offset >= [definitionString length])
        return 0;
    
    return [definitionString characterAtIndex:index + offset];
}

- (char)peek {
    return [self peek:1];
}

@end

@implementation EntityDefinitionTokenizer

- (id)initWithDefinitionString:(NSString *)theDefinitionString {
    if (self = [self init]) {
        definitionString = [theDefinitionString retain];
        index = -1;
        line = 1;
        column = 0;
        state = TS_OUTDEF;
        token = [[EntityDefinitionToken alloc] init];
    }
    
    return self;
}

- (EntityDefinitionToken *)nextToken {
    static int s, l;
    while ([self next]) {
        switch (state) {
            case TS_OUTDEF:
                switch (c) {
                    case '/':
                        if ([self peek] == '*') {
                            state = TS_INDEF;
                            while (c != ' ')
                                [self next];
                            return [token setType:TT_ED_O data:nil line:line column:column charsRead:index];
                        } else if ([self peek] == '/') {
                            state = TS_COM;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case TS_INDEF:
                switch (c) {
                    case '*':
                        if ([self peek] == '/') {
                            [self next];
                            state = TS_OUTDEF;
                            return [token setType:TT_ED_C data:nil line:line column:column charsRead:index];
                        }
                        break;
                    case '(':
                        return [token setType:TT_B_O data:nil line:line column:column charsRead:index];
                    case ')':
                        return [token setType:TT_B_C data:nil line:line column:column charsRead:index];
                    case '{':
                        return [token setType:TT_CB_O data:nil line:line column:column charsRead:index];
                    case '}':
                        return [token setType:TT_CB_C data:nil line:line column:column charsRead:index];
                    case ';':
                        return [token setType:TT_SC data:nil line:line column:column charsRead:index];
                    case '?':
                        return [token setType:TT_QM data:nil line:line column:column charsRead:index];
                    case '\n':
                        return [token setType:TT_NL data:nil line:line column:column charsRead:index];
                    case ',':
                        return [token setType:TT_C data:nil line:line column:column charsRead:index];
                    case ' ':
                    case '\t':
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
                        s = index;
                        l = 1;
                        break;
                    case '"':
                        state = TS_Q_STR;
                        s = index + 1;
                        l = 0;
                        break;
                    default:
                        state = TS_WORD;
                        s = index;
                        l = 1;
                        break;
                }
                break;
            case TS_COM:
                if (c == '\n')
                    state = TS_OUTDEF;
                break;
            case TS_WORD:
                switch (c) {
                    case '/':
                        if ([self peek] == '*') {
                            [self pushBack];
                        } else {
                            l++;
                            break;
                        }
                    case '(':
                    case ' ':
                    case '\n':
                    case '\t':
                        state = TS_INDEF;
                        [self pushBack];
                        NSString* word = [definitionString substringWithRange:NSMakeRange(s, l)];
                        return [token setType:TT_WORD data:word line:line column:column charsRead:index];
                    default:
                        l++;
                        break;
                }
                break;
            case TS_Q_STR:
                if (c == '"') {
                    state = TS_INDEF;
                    NSString* string = [definitionString substringWithRange:NSMakeRange(s, l)];
                    return [token setType:TT_STR data:string line:line column:column charsRead:index];
                } else {
                    l++;
                }
                break;
            case TS_DEC:
                if (c == '.')
                    state = TS_FRAC;
            case TS_FRAC: {
                switch (c) {
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
                    case '.':
                        l++;
                        break;
                    case ')':
                    case '\t':
                    case ',':
                    case ' ': {
                        if (state == TS_DEC) {
                            state = TS_INDEF;
                            NSString* string = [definitionString substringWithRange:NSMakeRange(s, l)];
                            NSNumber* number = [NSNumber numberWithInt:[string intValue]];
                            [self pushBack];
                            return [token setType:TT_DEC data:number line:line column:column charsRead:index];
                        } else {
                            state = TS_INDEF;
                            NSString* string = [definitionString substringWithRange:NSMakeRange(s, l)];
                            NSNumber* number = [NSNumber numberWithFloat:[string floatValue]];
                            [self pushBack];
                            return [token setType:TT_FRAC data:number line:line column:column charsRead:index];
                        }
                        break;
                    }
                    default:
                        state = TS_WORD;
                        break;
                }
                break;
            }            
            default:
                break;
        }
    }
    
    return nil;
}

- (EntityDefinitionToken *)peekToken {
    int oldLine = line;
    int oldColumn = column;
    int oldIndex = index;
    char oldC = c;
    ETokenizerState oldState = state;

    [self nextToken];
    
    line = oldLine;
    column = oldColumn;
    index = oldIndex;
    c = oldC;
    state = oldState;
    
    return token;
}

- (NSString *)remainderAsDescription {
    if (state != TS_INDEF)
        [NSException raise:NSInternalInconsistencyException format:@"must be inside a definition to obtain the remainder as a description string"];

    [self next];
    
    int s = index;
    int l = 0;
    
    while (state != TS_EOF && c != '*' && [self peek] != '/') {
        [self next];
        l++;
    }
    
    [self pushBack];
    return [definitionString substringWithRange:NSMakeRange(s, l)];
}

-  (void)dealloc {
    [definitionString release];
    [token release];
    [super dealloc];
}

@end

/*
 *  IO.c
 *  TrenchBroom
 *
 *  Created by Kristian Duske on 15.05.11.
 *  Copyright 2011 TU Berlin. All rights reserved.
 *
 */

#include "IO.h"

char readChar(NSData* data, int location) {
    char c;
    [data getBytes:&c range:NSMakeRange(location, 1)];
    return c;
}

NSString* readString(NSData* data, NSRange range) {
    NSData* strData = [data subdataWithRange:range];
    return [NSString stringWithCString:[strData bytes] encoding:NSASCIIStringEncoding];
}

unsigned int readInt(NSData* data, int location) {
    unsigned int result;
    [data getBytes:(void *)&result range:NSMakeRange(location, 4)];
    return NSSwapLittleIntToHost(result);
}

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

unsigned char readUChar(NSData* data, int location) {
    return (unsigned char)readChar(data, location);
}

NSString* readString(NSData* data, NSRange range) {
    NSData* strData = [data subdataWithRange:range];
    return [NSString stringWithCString:[strData bytes] encoding:NSASCIIStringEncoding];
}

unsigned int readUInt(NSData* data, int location) {
    unsigned int result;
    [data getBytes:(unsigned int *)&result range:NSMakeRange(location, 4)];
    return NSSwapLittleIntToHost(result);
}

int readInt(NSData* data, int location) {
    unsigned int result;
    [data getBytes:(unsigned int *)&result range:NSMakeRange(location, 4)];
    return NSSwapLittleIntToHost(result);
}

unsigned int readUShort(NSData* data, int location) {
    unsigned int result;
    [data getBytes:(unsigned int *)&result range:NSMakeRange(location, 2)];
    return NSSwapLittleIntToHost(result);
}

int readShort(NSData* data, int location) {
    unsigned int result;
    [data getBytes:(unsigned int *)&result range:NSMakeRange(location, 2)];
    return NSSwapLittleIntToHost(result);
}

float readFloat(NSData* data, int location) {
    NSSwappedFloat result;
    [data getBytes:(NSSwappedFloat *)&result range:NSMakeRange(location, 4)];
    return NSSwapLittleFloatToHost(result);
}

TVector3f readVector3f(NSData* data, int location) {
    TVector3f result;
    result.x = readFloat(data, location);
    result.y = readFloat(data, location + 4);
    result.z = readFloat(data, location + 8);
    return result;
}
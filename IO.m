/*
 *  IO.c
 *  TrenchBroom
 *
 *  Created by Kristian Duske on 15.05.11.
 *  Copyright 2011 TU Berlin. All rights reserved.
 *
 */

#include "IO.h"
#include "PakDirectoryEntry.h"

int const PAK_MAGIC_ADDRESS = 0x0;
int const PAK_MAGIC_SIZE = 0x4;
int const PAK_DIR_OFFSET_ADDRESS = 0x4;
int const PAK_DIR_SIZE_ADDRESS = 0x8;
int const PAK_ENTRY_SIZE = 0x40;
int const PAK_ENTRY_NAME_OFFSET = 0x0;
int const PAK_ENTRY_NAME_SIZE = 0x38;
int const PAK_ENTRY_ADDRESS_OFFSET = 0x38;
int const PAK_ENTRY_SIZE_OFFSET = 0x3C;

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

BOOL readPakDirectory(NSData* data, PakDirectory* directory) {
    NSString* magic = readString(data, NSMakeRange(PAK_MAGIC_ADDRESS, PAK_MAGIC_SIZE));
    if (![magic isEqualToString:@"PACK"])
        return NO;
    
    int dirOffset = readInt(data, PAK_DIR_OFFSET_ADDRESS);
    int dirSize = readInt(data, PAK_DIR_SIZE_ADDRESS);
    int numEntries = dirSize / PAK_ENTRY_SIZE;
    
    for (int i = 0; i < numEntries; i++) {
        int dirEntryAddress = dirOffset + i * dirSize;
        NSString* name = readString(data, NSMakeRange(dirEntryAddress + PAK_ENTRY_NAME_OFFSET, PAK_ENTRY_NAME_SIZE));
        int entryAddress = readInt(data, dirEntryAddress + PAK_ENTRY_ADDRESS_OFFSET);
        int entrySize = readInt(data, dirEntryAddress + PAK_ENTRY_SIZE_OFFSET);
        
        PakDirectoryEntry* entry = [[PakDirectoryEntry alloc] initWithName:name address:entryAddress size:entrySize];
        [directory addEntry:entry];
        [entry release];
    }
    
    return YES;
}
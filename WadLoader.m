//
//  WadLoader.m
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WadLoader.h"
#import "Wad.h"
#import "WadEntry.h"

NSString* const InvalidFileTypeException = @"InvalidFileTypeException";
NSString* const EndOfStreamException = @"EndOfStreamException";

int const WAD_TYPE_ADDRESS = 0;
int const WAD_TYPE_LENGTH = 4;
int const WAD_NUM_ENTRIES_ADDRESS = 4;
int const WAD_DIR_OFFSET_ADDRESS = 8;
int const WAD_DIR_ENTRY_LENGTH = 32;
int const WAD_DIR_ENTRY_DSIZE_OFFSET = 4;
int const WAD_DIR_ENTRY_TYPE_OFFSET = 12;
int const WAD_DIR_ENTRY_ADDRESS_OFFSET = 0;
int const WAD_DIR_ENTRY_NAME_OFFSET = 16;

@implementation WadLoader

- (NSString *)readString:(NSData *)data range:(NSRange)range {
    @try {
        [data getBytes:buffer range:range];
    }
    @catch (NSException * e) {
        [NSException raise:EndOfStreamException format:@"read past end of wad file"];
    }
    return [NSString stringWithCString:(char*)buffer encoding:NSASCIIStringEncoding];
}

- (unsigned int)readInt:(NSData *)data location:(int)location {
    @try {
        [data getBytes:buffer range:NSMakeRange(location, 4)];
    }
    @catch (NSException * e) {
        [NSException raise:EndOfStreamException format:@"read past end of wad file"];
    }
    
    unsigned int result = 0;
    result |= buffer[3] << 3 * 8;
    result |= buffer[2] << 2 * 8;
    result |= buffer[1] << 1 * 8;
    result |= buffer[0] << 0 * 8;
    
    return NSSwapLittleIntToHost(result);
}

- (char)readChar:(NSData *)data location:(int)location {
    @try {
        [data getBytes:buffer range:NSMakeRange(location, 1)];
    }
    @catch (NSException * e) {
        [NSException raise:EndOfStreamException format:@"read past end of wad file"];
    }
    
    return (char) buffer[0];
}

- (EWadEntryType)getType:(char)typeCh {
    switch (typeCh) {
        case '@':
            return WT_PAL;
        case 'B':
            return WT_SPIC;
        case 'D':
            return WT_MIP;
        case 'E':
            return WT_CPIC;
        default:
            [NSException raise:NSInvalidArgumentException format:@"unknown wad entry type char: %c", typeCh];
    }
    
    return 0; // never reached
}

- (void)readEntry:(NSData *)data location:(unsigned int)location wad:(Wad *)wad {
    unsigned int address = [self readInt:data location:location + WAD_DIR_ENTRY_ADDRESS_OFFSET];
    unsigned int dsize = [self readInt:data location:location + WAD_DIR_ENTRY_DSIZE_OFFSET];
    char type = [self readChar:data location:location + WAD_DIR_ENTRY_TYPE_OFFSET];
    NSString* name = [self readString:data range:NSMakeRange(location + WAD_DIR_ENTRY_NAME_OFFSET, 16)];
    NSData* entryData = [data subdataWithRange:NSMakeRange(address, dsize)];
    
    [wad createEntryWithType:[self getType:type] name:name data:entryData];
}

- (Wad *)loadFromData:(NSData *)someData wadName:(NSString *)wadName {
    if (someData == nil)
        [NSException raise:NSInvalidArgumentException format:@"data must not be nil"];
    if (wadName == nil)
        [NSException raise:NSInvalidArgumentException format:@"wad name must not be nil"];

    NSString* fileType = [self readString:someData range:NSMakeRange(WAD_TYPE_ADDRESS, WAD_TYPE_LENGTH)];
    if (![fileType isEqualToString:@"WAD2"])
        [NSException raise:InvalidFileTypeException format:@"WAD file header is corrupt"];
    
    unsigned int numEntries = [self readInt:someData location:WAD_NUM_ENTRIES_ADDRESS];
    unsigned int dirOffset = [self readInt:someData location:WAD_DIR_OFFSET_ADDRESS];
    
    Wad* wad = [[Wad alloc] initWithName:wadName];
    unsigned int address = dirOffset;
    for (int i = 0; i < numEntries; i++) {
        [self readEntry:someData location:address wad:wad];
        address += WAD_DIR_ENTRY_LENGTH;
    }
    return [wad autorelease];
}

@end

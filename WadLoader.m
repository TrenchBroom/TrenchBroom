//
//  WadLoader.m
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WadLoader.h"
#import "Wad.h"
#import "WadPaletteEntry.h"
#import "WadTextureEntry.h"

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
int const WAD_PAL_LENGTH = 3 * 256;
int const WAD_TEX_WIDTH_OFFSET = 16;
int const WAD_TEX_HEIGHT_OFFSET = 20;
int const WAD_TEX_MIP0_OFFSET = 24;
int const WAD_TEX_MIP1_OFFSET = 28;
int const WAD_TEX_MIP2_OFFSET = 32;
int const WAD_TEX_MIP3_OFFSET = 36;

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

- (void)readEntry:(NSData *)data location:(unsigned int)location wad:(Wad *)wad {
    unsigned int address = [self readInt:data location:location + WAD_DIR_ENTRY_ADDRESS_OFFSET];
    char type = [self readChar:data location:location + WAD_DIR_ENTRY_TYPE_OFFSET];
    NSString* name = [self readString:data range:NSMakeRange(location + WAD_DIR_ENTRY_NAME_OFFSET, 16)];
    
    switch (type) {
        case '@': {
            NSData* paletteData = [data subdataWithRange:NSMakeRange(address, WAD_PAL_LENGTH)];
            WadPaletteEntry* paletteEntry = [[WadPaletteEntry alloc] initWithName:name data:paletteData];
            [wad addPaletteEntry:paletteEntry];
            [paletteEntry release];
            break;
        }
        case 'D': {
            unsigned int width = [self readInt:data location:address + WAD_TEX_WIDTH_OFFSET];
            unsigned int height = [self readInt:data location:address + WAD_TEX_HEIGHT_OFFSET];
            unsigned int mip0Offset = [self readInt:data location:address + WAD_TEX_MIP0_OFFSET];
            unsigned int mip1Offset = [self readInt:data location:address + WAD_TEX_MIP1_OFFSET];
            unsigned int mip2Offset = [self readInt:data location:address + WAD_TEX_MIP2_OFFSET];
            unsigned int mip3Offset = [self readInt:data location:address + WAD_TEX_MIP3_OFFSET];
            NSData* mip0 = [data subdataWithRange:NSMakeRange(address + mip0Offset, width * height)];
            NSData* mip1 = [data subdataWithRange:NSMakeRange(address + mip1Offset, width * height / 4)];
            NSData* mip2 = [data subdataWithRange:NSMakeRange(address + mip2Offset, width * height / 16)];
            NSData* mip3 = [data subdataWithRange:NSMakeRange(address + mip3Offset, width * height / 64)];
            
            WadTextureEntry* textureEntry = [[WadTextureEntry alloc] initWithName:name 
                                                                            width:width 
                                                                           height:height 
                                                                             mip0:mip0 
                                                                             mip1:mip1 
                                                                             mip2:mip2 
                                                                             mip3:mip3];
            [wad addTextureEntry:textureEntry];
            [textureEntry release];
            break;
        }
        default:
            // ignore
            break;
    }
}

- (Wad *)loadFromData:(NSData *)someData wadName:(NSString *)wadName {
    if (someData == nil)
        [NSException raise:NSInvalidArgumentException format:@"data must not be nil"];
    if (wadName == nil)
        [NSException raise:NSInvalidArgumentException format:@"wad name must not be nil"];

    NSDate* startDate = [NSDate date];
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
    
    NSLog(@"Loaded '%@' in %f seconds", wadName, -[startDate timeIntervalSinceNow]);
    
    return [wad autorelease];
}

@end

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

#import "WadLoader.h"
#import "IO.h"
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

- (void)readEntry:(NSData *)data location:(unsigned int)location wad:(Wad *)wad {
    unsigned int address = readLong(data, location + WAD_DIR_ENTRY_ADDRESS_OFFSET);
    char type = readChar(data, location + WAD_DIR_ENTRY_TYPE_OFFSET);
    NSString* name = readString(data, NSMakeRange(location + WAD_DIR_ENTRY_NAME_OFFSET, 16));
    
    switch (type) {
        case '@': {
            NSData* paletteData = [data subdataWithRange:NSMakeRange(address, WAD_PAL_LENGTH)];
            WadPaletteEntry* paletteEntry = [[WadPaletteEntry alloc] initWithName:name data:paletteData];
            [wad addPaletteEntry:paletteEntry];
            [paletteEntry release];
            break;
        }
        case 'D': {
            int width = readLong(data, address + WAD_TEX_WIDTH_OFFSET);
            int height = readLong(data, address + WAD_TEX_HEIGHT_OFFSET);
            int mip0Offset = readLong(data, address + WAD_TEX_MIP0_OFFSET);
            int mip1Offset = readLong(data, address + WAD_TEX_MIP1_OFFSET);
            int mip2Offset = readLong(data, address + WAD_TEX_MIP2_OFFSET);
            int mip3Offset = readLong(data, address + WAD_TEX_MIP3_OFFSET);
            
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
    NSDate* startDate = [NSDate date];
    NSString* fileType = readString(someData, NSMakeRange(WAD_TYPE_ADDRESS, WAD_TYPE_LENGTH));
    if (![fileType isEqualToString:@"WAD2"])
        [NSException raise:InvalidFileTypeException format:@"WAD file header is corrupt"];

    int numEntries = readLong(someData, WAD_NUM_ENTRIES_ADDRESS);
    int dirOffset = readLong(someData, WAD_DIR_OFFSET_ADDRESS);
    
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

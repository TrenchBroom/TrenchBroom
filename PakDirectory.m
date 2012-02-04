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

#import "PakDirectory.h"
#import "PakDirectoryEntry.h"
#import "IO.h"

int const PAK_HEADER_ADDRESS = 0x0;
int const PAK_HEADER_SIZE = 0x40;
int const PAK_MAGIC_OFFSET = 0x0;
int const PAK_MAGIC_SIZE = 0x4;
int const PAK_DIR_OFFSET_OFFSET = 0x4;
int const PAK_DIR_SIZE_OFFSET = 0x8;

int const PAK_ENTRY_SIZE = 0x40;
int const PAK_ENTRY_NAME_OFFSET = 0x0;
int const PAK_ENTRY_NAME_SIZE = 0x38;
int const PAK_ENTRY_ADDRESS_OFFSET = 0x38;
int const PAK_ENTRY_SIZE_OFFSET = 0x3C;

@implementation PakDirectory

- (id)initWithPath:(NSString *)thePath {
    NSAssert(thePath != nil, @"pak file path must not be nil");
    
    if ((self = [self init])) {
        path = [thePath retain];
        handle = [[NSFileHandle fileHandleForReadingAtPath:path] retain];
        
        if (handle == nil) {
            NSLog(@"Cannot open pak file %@", path);
            [self release];
            return nil;
        }
        
        [handle seekToFileOffset:PAK_HEADER_ADDRESS];
        NSData* headerData = [handle readDataOfLength:PAK_HEADER_SIZE];
        
        NSString* magic = readString(headerData, NSMakeRange(PAK_MAGIC_OFFSET, PAK_MAGIC_SIZE));
        if (![magic isEqualToString:@"PACK"]) {
            NSLog(@"Cannot read pak file %@: magic does not match", path);
            [self release];
            return nil;
        }
        
        int dirOffset = readLong(headerData, PAK_DIR_OFFSET_OFFSET);
        int dirSize = readLong(headerData, PAK_DIR_SIZE_OFFSET);
        int numEntries = dirSize / PAK_ENTRY_SIZE;
        
        entries = [[NSMutableDictionary alloc] initWithCapacity:numEntries];

        [handle seekToFileOffset:dirOffset];
        NSData* directoryData = [handle readDataOfLength:dirSize];
        
        for (int i = 0; i < numEntries; i++) {
            int entryOffset = i * PAK_ENTRY_SIZE;
            NSString* name = readString(directoryData, NSMakeRange(entryOffset + PAK_ENTRY_NAME_OFFSET, PAK_ENTRY_NAME_SIZE));
            int entryAddress = readLong(directoryData, entryOffset + PAK_ENTRY_ADDRESS_OFFSET);
            int entrySize = readLong(directoryData, entryOffset + PAK_ENTRY_SIZE_OFFSET);
            
            PakDirectoryEntry* entry = [[PakDirectoryEntry alloc] initWithName:name address:entryAddress size:entrySize];
            [entries setObject:entry forKey:name];
            [entry release];
        }
        
    }
    
    return self;
}

- (void)dealloc {
    [path release];
    [entries release];
    [handle release];
    [super dealloc];
}

- (NSString *)path {
    return path;
}

- (NSData *)entryForName:(NSString *)theName {
    NSAssert(theName != nil, @"name must not be nil");
    
    PakDirectoryEntry* entry = [entries objectForKey:theName];
    if (entry == nil)
        return nil;
    
    return [entry entryDataFromHandle:handle];
}

- (NSComparisonResult)compareByName:(PakDirectory *)other {
    NSString* name = [path lastPathComponent];
    NSString* otherName = [[other path] lastPathComponent];
    return [name caseInsensitiveCompare:otherName];
}

@end

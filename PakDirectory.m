//
//  PakDirectory.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

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
    
    if (self = [self init]) {
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
        
        int dirOffset = readInt(headerData, PAK_DIR_OFFSET_OFFSET);
        int dirSize = readInt(headerData, PAK_DIR_SIZE_OFFSET);
        int numEntries = dirSize / PAK_ENTRY_SIZE;
        
        [handle seekToFileOffset:dirOffset];
        NSData* directoryData = [handle readDataOfLength:dirSize];
        
        for (int i = 0; i < numEntries; i++) {
            int entryOffset = i * PAK_ENTRY_SIZE;
            NSString* name = readString(directoryData, NSMakeRange(entryOffset + PAK_ENTRY_NAME_OFFSET, PAK_ENTRY_NAME_SIZE));
            int entryAddress = readInt(directoryData, entryOffset + PAK_ENTRY_ADDRESS_OFFSET);
            int entrySize = readInt(directoryData, entryOffset + PAK_ENTRY_SIZE_OFFSET);
            
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

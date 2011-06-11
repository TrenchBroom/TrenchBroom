//
//  PakDirectoryEntry.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface PakDirectoryEntry : NSObject {
    NSString* name;
    int address;
    int size;
}

- (id)initWithName:(NSString *)theName address:(int)theAddress size:(int)theSize;

- (NSString *)name;
- (NSData *)entryDataFromHandle:(NSFileHandle *)theHandle;

@end

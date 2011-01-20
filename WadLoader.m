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

NSString* const EndOfStreamException = @"EndOfStreamException";

@implementation WadLoader

- (int)readInt:(NSInputStream *)stream {
    if ([stream read:intBuffer maxLength:4] != 4)
        [NSException raise:EndOfStreamException format:@"read past end of wad file"];
    
    
}

- (Wad *)loadFromData:(NSData *)someData {
    if (someData == nil)
        [NSException raise:NSInvalidArgumentException format:@"data must not be nil"];
    
    NSInputStream* stream = [[NSInputStream alloc] initWithData:someData];
    [stream open];
    @try {
        
    }
    @finally {
        [stream close];
        [stream release];
    }
}

@end

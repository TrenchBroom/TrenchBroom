//
//  WadEntry.h
//  TrenchBroom
//
//  Created by Kristian Duske on 20.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef enum {
    WT_PAL, // palette
    WT_SPIC, // status bar picture
    TS_MIP, // mip texture
    TS_CPIC // console picture
} EWadEntryType;


@interface WadEntry : NSObject {
    @private
    EWadEntryType type;
    NSString* name;
    NSData* data;
}

- (id)initWithType:(EWadEntryType)aType name:(NSString *)aName data:(NSData *)someData;

- (EWadEntryType)type;
- (NSString *)name;
- (NSData *)data;

@end

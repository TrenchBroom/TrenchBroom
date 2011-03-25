//
//  WadTextureEntry.m
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WadTextureEntry.h"


@implementation WadTextureEntry
- (id)initWithName:(NSString *)theName width:(int)theWidth height:(int)theHeight mip0:(NSData *)mip0Data mip1:(NSData *)mip1Data mip2:(NSData *)mip2Data mip3:(NSData *)mip3Data {
    if (self = [super initWithName:theName]) {
        width = theWidth;
        height = theHeight;
        mip0 = [mip0Data retain];
        mip1 = [mip1Data retain];
        mip2 = [mip2Data retain];
        mip3 = [mip3Data retain];
    }
    
    return self;
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

- (NSData *)mip0 {
    return mip0;
}

- (NSData *)mip1 {
    return mip1;
}

- (NSData *)mip2 {
    return mip2;
}

- (NSData *)mip3 {
    return mip3;
}

- (void)dealloc {
    [mip0 release];
    [mip1 release];
    [mip2 release];
    [mip3 release];
    [super dealloc];
}

@end

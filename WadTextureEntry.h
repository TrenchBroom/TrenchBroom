//
//  WadTextureEntry.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "WadEntry.h"

@interface WadTextureEntry : WadEntry {
    @private
    int width;
    int height;
    NSData* mip0;
    NSData* mip1;
    NSData* mip2;
    NSData* mip3;
}
- (id)initWithName:(NSString *)theName width:(int)theWidth height:(int)theHeight mip0:(NSData *)mip0Data mip1:(NSData *)mip1Data mip2:(NSData *)mip2Data mip3:(NSData *)mip3Data;

- (int)width;
- (int)height;

- (NSData *)mip0;
- (NSData *)mip1;
- (NSData *)mip2;
- (NSData *)mip3;

@end

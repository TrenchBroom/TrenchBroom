//
//  IntData.m
//  TrenchBroom
//
//  Created by Kristian Duske on 31.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "IntData.h"


@implementation IntData
- (id)init {
    if ((self = [super init])) {
        data = [[NSMutableData alloc] init];
        count = 0;
    }
    
    return self;
}

- (id)initDataWithCapacity:(int)capacity {
    if ((self = [super init])) {
        data = [[NSMutableData alloc] initWithLength:capacity * sizeof(float)];
        count = 0;
    }
    
    return self;
}

- (void)appendInt:(int)value {
    [data appendBytes:(char *)&value length:sizeof(int)];
    count++;
}

- (const void*)bytes{
    return [data bytes];
}

- (int)count {
    return count;
}

- (void)dealloc {
    [data release];
    [super dealloc];
}
@end

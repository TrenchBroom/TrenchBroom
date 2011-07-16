//
//  BspTexture.m
//  TrenchBroom
//
//  Created by Kristian Duske on 16.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "BspTexture.h"


@implementation BspTexture

- (id)initWithName:(NSString *)theName image:(NSData *)theImage width:(int)theWidth height:(int)theHeight {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theImage != nil, @"image must not be nil");
    NSAssert(theWidth > 0, @"width must be a positive integer");
    NSAssert(theHeight > 0, @"height must be a positive integer");
    
    if ((self = [self init])) {
        name = [theName retain];
        image = [theImage retain];
        width = theWidth;
        height = theHeight;
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [image release];
    [super dealloc];
}

- (NSString *)name; {
    return name;
}

- (NSData *)image {
    return image;
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

@end

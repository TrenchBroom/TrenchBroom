//
//  BspTexture.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface BspTexture : NSObject {
@private
    NSString* name;
    int width;
    int height;
    NSData* image;
}

- (id)initWithName:(NSString *)theName image:(NSData *)theImage width:(int)theWidth height:(int)theHeight;

- (NSString *)name;
- (NSData *)image;
- (int)width;
- (int)height;

@end

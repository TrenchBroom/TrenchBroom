//
//  EntityModelSkin.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface AliasSkin : NSObject {
    int width;
    int height;
    float* times;
    NSArray* pictures;
}

- (id)initSingleSkin:(NSData *)thePicture width:(int)theWidth height:(int)theHeight;
- (id)initMultiSkin:(NSArray *)thePictures times:(float *)theTimes width:(int)theWidth height:(int)theHeight;

- (int)width;
- (int)height;

- (NSData *)pictureAtIndex:(int)theIndex;
- (float)timeAtIndex:(int)theIndex;
- (int)pictureCount;

@end

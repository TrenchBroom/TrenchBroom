//
//  BspManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class Bsp;

@interface BspManager : NSObject {
@private
    NSMutableDictionary* bsps;
    NSData* palette;
}

+ (BspManager *)sharedManager;

- (Bsp *)bspWithName:(NSString *)theName paths:(NSArray *)thePaths;

@end

//
//  SpawnFlag.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface SpawnFlag : NSObject {
@private
    int flag;
    NSString* name;
}

- (id)initWithName:(NSString *)theName flag:(int)theFlag;

- (NSString *)name;
- (int)flag;

- (NSComparisonResult)compareByFlag:(SpawnFlag *)otherFlag;
@end

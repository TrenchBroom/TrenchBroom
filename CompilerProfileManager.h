//
//  CompilerManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

@class CompilerProfile;

@interface CompilerProfileManager : NSObject {
    NSMutableArray* profiles;
}

+ (CompilerProfileManager *)sharedManager;

- (void)updateDefaults;

- (NSArray *)profiles;
- (void)insertObject:(CompilerProfile *)theProfile inProfilesAtIndex:(NSUInteger)theIndex;
- (void)removeObjectFromProfilesAtIndex:(NSUInteger)theIndex;

@end

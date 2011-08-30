//
//  CompilerManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

@class CompilerTool;
@class CompilerProfile;

@interface CompilerManager : NSObject {
    NSMutableArray* tools;
    NSMutableArray* profiles;
}

+ (CompilerManager *)sharedManager;

- (void)updateDefaults;

- (NSArray *)tools;
- (void)insertObject:(CompilerTool *)theTool inToolsAtIndex:(NSUInteger)theIndex;
- (void)removeObjectFromToolsAtIndex:(NSUInteger)theIndex;

/*
- (NSArray *)profiles;
- (void)insertObject:(CompilerProfile *)theProfile inProfilesAtIndex:(NSUInteger)theIndex;
- (void)removeObjectFromProfilesAtIndex:(NSUInteger)theIndex;
*/
@end

//
//  Observable.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Observable : NSObject {
    @private
    NSMutableArray* observers;
}

- (void)addObserver:(id)target selector:(SEL)selector name:(NSString *)name;
- (void)removeObserver:(id)target;
- (void)removeObserver:(id)target name:(NSString *)name;

- (void)notifyObservers:(NSString *)name;
- (void)notifyObservers:(NSString *)name infoObject:(id)infoObject infoKey:(NSString *)infoKey;
- (void)notifyObservers:(NSString *)name userInfo:(NSDictionary *)userInfo;

- (void)addForward:(NSString *)name from:(Observable *)observable;
- (void)forwardNotification:(NSNotification *)notification;

- (BOOL)postNotifications;
@end

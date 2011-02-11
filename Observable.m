//
//  Observable.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"
#import "Observer.h"

@implementation Observable

- (id)init {
    if (self = [super init]) {
        observers = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)addObserver:(id)target selector:(SEL)selector name:(NSString *)name {
    Observer* observer = [[Observer alloc] initWithTarget:target selector:selector name:name];
    [observers addObject:observer];
    [observer release];
}

- (void)removeObserver:(id)target {
    for (int i = 0; i < [observers count]; i++) {
        Observer* observer = [observers objectAtIndex:i];
        if ([observer target] == target)
            [observers removeObjectAtIndex:i--];
    }
}

- (void)removeObserver:(id)target name:(NSString *)name {
    for (int i = 0; i < [observers count]; i++) {
        Observer* observer = [observers objectAtIndex:i];
        if ([observer target] == target && [observer name] == name)
            [observers removeObjectAtIndex:i--];
    }
}

- (void)notifyObservers:(NSString *)name{
    if (![self postNotifications])
        return;
    
    if ([observers count] == 0)
        return;

    [self notifyObservers:name userInfo:nil];
}

- (void)notifyObservers:(NSString *)name infoObject:(id)infoObject infoKey:(NSString *)infoKey{
    if (![self postNotifications])
        return;
    
    if ([observers count] == 0)
        return;

    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:infoObject forKey:infoKey];
    [self notifyObservers:name userInfo:userInfo];
}

- (void)notifyObservers:(NSString *)name userInfo:(NSDictionary *)userInfo{
    if (![self postNotifications])
        return;

    if ([observers count] == 0)
        return;
    
    NSNotification* notification = [NSNotification notificationWithName:name object:self userInfo:userInfo];
    NSEnumerator* observerEn = [observers objectEnumerator];
    Observer* observer;
    while ((observer = [observerEn nextObject]))
        if ([observer name] == name)
            [observer notify:notification];
}

- (void)addForward:(NSString *)name from:(Observable *)observable {
    [observable addObserver:self  selector:@selector(forwardNotification:) name:name];
}

- (void)forwardNotification:(NSNotification *)notification {
    NSString* name = [notification name];

    if ([observers count] > 0) {
        NSEnumerator* observerEn = [observers objectEnumerator];
        Observer* observer;
        while ((observer = [observerEn nextObject]))
            if ([observer name] == name)
                [observer notify:notification];
    }
}

- (BOOL)postNotifications {
    return YES;
}

- (void)dealloc {
    [observers release];
    [super dealloc];
}

@end

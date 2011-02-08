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
        observers = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)addObserver:(id)target selector:(SEL)selector name:(NSString *)name {
    NSMutableArray* observersForName = [observers objectForKey:name];
    if (observersForName == nil) {
        observersForName = [[NSMutableArray alloc] init];
        [observers setObject:observersForName forKey:name];
        [observersForName release];
    }

    Observer* observer = [[Observer alloc] initWithTarget:target selector:selector];
    [observersForName addObject:observer];
    [observer release];
}

- (void)removeObserver:(id)target {
    NSEnumerator* en = [observers objectEnumerator];
    NSMutableArray* observersForName;
    while ((observersForName = [en nextObject])) {
        for (int i = 0; i < [observersForName count]; i++) {
            Observer* observer = [observersForName objectAtIndex:i];
            if ([observer target] == target)
                [observersForName removeObjectAtIndex:i--];
        }
    }
}

- (void)removeObserver:(id)target name:(NSString *)name {
    NSMutableArray* observersForName = [observers objectForKey:name];
    if (observersForName != nil) {
        for (int i = 0; i < [observersForName count]; i++) {
            Observer* observer = [observersForName objectAtIndex:i];
            if ([observer target] == target)
                [observersForName removeObjectAtIndex:i--];
        }
    }
}

- (void)notifyObservers:(NSString *)name{
    if (![self postNotifications])
        return;
    
    [self notifyObservers:name userInfo:nil];
}

- (void)notifyObservers:(NSString *)name infoObject:(id)infoObject infoKey:(NSString *)infoKey{
    if (![self postNotifications])
        return;
    
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:infoObject forKey:infoKey];
    [self notifyObservers:name userInfo:userInfo];
}

- (void)notifyObservers:(NSString *)name userInfo:(NSDictionary *)userInfo{
    if (![self postNotifications])
        return;
    
    NSArray* observersForName = [observers objectForKey:name];
    if (observersForName != nil && [observersForName count] > 0) {
        NSNotification* notification = [NSNotification notificationWithName:name object:self userInfo:userInfo];
        NSEnumerator* observerEn = [observersForName objectEnumerator];
        Observer* observer;
        while ((observer = [observerEn nextObject]))
            [observer notify:notification];
    }
}

- (void)addForward:(NSString *)name from:(Observable *)observable {
    [observable addObserver:self  selector:@selector(forwardNotification:) name:name];
}

- (void)forwardNotification:(NSNotification *)notification {
    NSString* name = [notification name];
    NSArray* observersForName = [observers objectForKey:name];

    if (observersForName != nil) {
        NSEnumerator* observerEn = [observersForName objectEnumerator];
        Observer* observer;
        while ((observer = [observerEn nextObject]))
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

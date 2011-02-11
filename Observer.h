//
//  Observer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Observer : NSObject {
    @private
    id target;
    SEL selector;
    NSString* name;
}

- (id)initWithTarget:(id)theTarget selector:(SEL)theSelector name:(NSString *)theName;
- (void)notify:(NSNotification *)notification;

- (id)target;
- (NSString *)name;
@end

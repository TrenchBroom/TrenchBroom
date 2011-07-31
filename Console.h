//
//  Console.h
//  TrenchBroom
//
//  Created by Kristian Duske on 31.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Console : NSObject {
    NSTextView* textView;
    NSDictionary* regularAttrs;
    NSDictionary* boldAttrs;
}

+ (Console *)sharedConsole;

- (id)initWithTextView:(NSTextView *)theTextView;

- (void)log:(NSString *)theString;
- (void)logBold:(NSString *)theString;

@end

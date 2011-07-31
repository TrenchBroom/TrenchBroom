//
//  Console.m
//  TrenchBroom
//
//  Created by Kristian Duske on 31.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "Console.h"

static Console* sharedInstance = nil;

@interface Console (private)

- (void)log:(NSString *)theString bold:(BOOL)isBold;

@end

@implementation Console (private)

- (void)log:(NSString *)theString bold:(BOOL)isBold {
    NSAssert(theString != nil, @"string must not be nil");

    NSAttributedString* attributedString;
    if (isBold)
        attributedString = [[NSAttributedString alloc] initWithString:theString attributes:boldAttrs];
    else
        attributedString = [[NSAttributedString alloc] initWithString:theString attributes:regularAttrs];
    
    [[textView textStorage] appendAttributedString:attributedString];
    [textView scrollToEndOfDocument:self];
    [attributedString release];
}

@end

@implementation Console

+ (Console *)sharedConsole {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

- (id)initWithTextView:(NSTextView *)theTextView {
    NSAssert(theTextView != nil, @"text view must not be nil");
    
    if ((self = [self init])) {
        textView = [theTextView retain];
        
        NSFont* regularFont = [NSFont fontWithName:@"Menlo" size:10];
        regularAttrs = [[NSDictionary alloc] initWithObjectsAndKeys:regularFont, NSFontAttributeName, nil];
        
        NSFontManager* fontManager = [NSFontManager sharedFontManager];
        NSFont* boldFont = [fontManager convertFont:regularFont toHaveTrait:NSBoldFontMask];
        boldAttrs = [[NSDictionary alloc] initWithObjectsAndKeys:boldFont, NSFontAttributeName, nil];
    }
    
    return self;
}

- (void)dealloc {
    [regularAttrs release];
    [boldAttrs release];
    [textView release];
    [super dealloc];
}

- (void)log:(NSString *)theString {
    Console* sharedConsole = [Console sharedConsole];
    if (self == sharedConsole) {
        NSLog(@"%@", theString);
    } else {
        [self log:theString bold:NO];
    }
}

- (void)logBold:(NSString *)theString {
    Console* sharedConsole = [Console sharedConsole];
    if (self == sharedConsole) {
        NSLog(@"%@", theString);
    } else {
        [self log:theString bold:YES];
    }
}

@end

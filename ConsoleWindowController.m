//
//  ConsoleWindowController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "ConsoleWindowController.h"

@implementation ConsoleWindowController

- (id)initWithWindowNibName:(NSString *)windowNibName {
    if ((self = [super initWithWindowNibName:windowNibName])) {
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
    [super dealloc];
}

- (void)log:(NSString *)theString bold:(BOOL)isBold {
    NSAssert(theString != nil, @"string must not be nil");
    
    NSAttributedString* attributedString;
    if (isBold)
        attributedString = [[NSAttributedString alloc] initWithString:theString attributes:boldAttrs];
    else
        attributedString = [[NSAttributedString alloc] initWithString:theString attributes:regularAttrs];
    
    [[consoleTextView textStorage] appendAttributedString:attributedString];
    [consoleTextView scrollToEndOfDocument:self];
    [attributedString release];
    
    if (![[self window] isVisible])
        [[self window] orderFront:self];
}

- (void)log:(NSString *)theString {
    [self log:theString bold:NO];
}

- (void)logBold:(NSString *)theString {
    [self log:theString bold:YES];
}

- (IBAction)clearConsole:(id)sender {
    NSAttributedString* emptyString = [[NSAttributedString alloc] initWithString:@""];
    [[consoleTextView textStorage] setAttributedString:emptyString];
    [emptyString release];
}

@end

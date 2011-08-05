//
//  ConsoleWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ConsoleWindowController : NSWindowController {
    IBOutlet NSTextView* consoleTextView;
    NSDictionary* regularAttrs;
    NSDictionary* boldAttrs;
}

- (IBAction)clearConsole:(id)sender;
- (void)log:(NSString *)theString bold:(BOOL)isBold;
- (void)log:(NSString *)theString;
- (void)logBold:(NSString *)theString;


@end

//
//  ProgressWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 31.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface ProgressWindowController : NSWindowController {
    IBOutlet NSProgressIndicator* progressIndicator;
    IBOutlet NSTextField* label;
}

- (NSProgressIndicator *)progressIndicator;
- (NSTextField *)label;

@end
